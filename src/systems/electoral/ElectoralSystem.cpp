/**
 * GPS - Geopolitical Simulator
 * ElectoralSystem.cpp
 */

#include "systems/electoral/ElectoralSystem.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace GPS {

ElectoralSystem::ElectoralSystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void ElectoralSystem::init() {
    std::cout << "[Electoral] Initializing electoral system..." << std::endl;

    for (auto id : world_.getAllCountryIds()) {
        auto& c = world_.getCountry(id);
        if (c.government == GovernmentType::PRESIDENTIAL_REPUBLIC ||
            c.government == GovernmentType::PARLIAMENTARY_REPUBLIC ||
            c.government == GovernmentType::CONSTITUTIONAL_MONARCHY ||
            c.government == GovernmentType::SEMI_PRESIDENTIAL) {
            SimDate nextDate;
            nextDate.year = 2028; nextDate.month = 10; nextDate.day = 1;
            scheduledElections_[id] = nextDate;
            electionTypes_[id] = "presidential";
        }
    }
}

void ElectoralSystem::update(double deltaTime, const SimDate& currentDate) {
    if (currentDate.hour != 0) return;

    for (auto countryId : world_.getAllCountryIds()) {
        auto it = scheduledElections_.find(countryId);
        if (it != scheduledElections_.end()) {
            if (currentDate.year == it->second.year &&
                currentDate.month == it->second.month &&
                currentDate.day == it->second.day) {
                processElection(countryId, currentDate);
                SimDate next = it->second;
                next.year += 4;
                it->second = next;
            }
        }

        updatePartyPopularity(world_.getCountry(countryId), deltaTime);
    }

    updateCampaigns(deltaTime);

    // Pesquisas semanais
    if (currentDate.day % 7 == 0) {
        generatePolls(currentDate);
    }

    // Processar referendos
    for (auto& ref : referendums_) {
        if (!ref.completed && currentDate.year == ref.date.year &&
            currentDate.month == ref.date.month && currentDate.day == ref.date.day) {
            processReferendum(ref, currentDate);
        }
    }
}

void ElectoralSystem::shutdown() {
    std::cout << "[Electoral] Shutting down." << std::endl;
}

void ElectoralSystem::processElection(CountryID countryId, const SimDate& date) {
    auto& country = world_.getCountry(countryId);
    auto partyIds = world_.getPartiesOfCountry(countryId);
    if (partyIds.empty()) return;

    ElectionResult result;
    result.date = date;
    result.country = countryId;
    result.type = electionTypes_.count(countryId) ? electionTypes_[countryId] : "presidential";

    // Turnout
    result.turnout = 0.6 + country.democracy * 0.1 + country.hdi * 0.05;
    if (country.internalStability < 0.3) result.turnout -= 0.15;
    result.turnout += RandomEngine::instance().randDouble(-0.05, 0.05);
    result.turnout = std::clamp(result.turnout, 0.2, 0.95);

    double totalShare = 0;
    for (auto pid : partyIds) {
        auto& party = world_.getParty(pid);
        double share = party.popularity;
        if (party.inPower) {
            share += country.governmentApproval * 0.15;
            if (country.gdpGrowthRate > 0.02) share += 0.05;
            if (country.gdpGrowthRate < -0.01) share -= 0.10;
            if (country.unemploymentRate > 0.10) share -= 0.08;
        }
        auto cit = campaigns_.find(countryId);
        if (cit != campaigns_.end()) {
            auto pit = cit->second.find(pid);
            if (pit != cit->second.end()) {
                share += pit->second.popularity * 0.05 + pit->second.momentum * 0.03;
            }
        }
        share += RandomEngine::instance().randDouble(-0.05, 0.05);
        share = std::max(0.01, share);
        result.voteShare[pid] = share;
        totalShare += share;
    }

    PartyID winner = 0;
    double bestShare = 0;
    if (totalShare > 0) {
        for (auto& [pid, share] : result.voteShare) {
            share /= totalShare;
            result.seatsWon[pid] = static_cast<int>(share * 100);
            if (share > bestShare) { bestShare = share; winner = pid; }
        }
    }
    result.winner = winner;
    result.fraudLevel = std::max(0.0, country.corruption - 0.5) * 0.5;
    if (country.democracy < 0.3) result.fraudLevel += 0.2;
    result.contested = result.fraudLevel > 0.15 && RandomEngine::instance().chance(0.3);

    electionHistory_[countryId].push_back(result);
    applyElectionResults(countryId, result);
}

void ElectoralSystem::applyElectionResults(CountryID countryId, const ElectionResult& result) {
    auto& country = world_.getCountry(countryId);
    for (auto pid : world_.getPartiesOfCountry(countryId)) {
        auto& party = world_.getParty(pid);
        party.inPower = (pid == result.winner);
        auto vit = result.voteShare.find(pid);
        if (vit != result.voteShare.end()) party.popularity = vit->second;
    }
    country.rulingPartyId = result.winner;
    if (result.voteShare.count(result.winner))
        country.governmentApproval = result.voteShare.at(result.winner);
    country.internalStability += result.contested ? -0.10 : 0.05;
    campaigns_.erase(countryId);
}

void ElectoralSystem::updateCampaigns(double dt) {
    for (auto& [countryId, partyCampaigns] : campaigns_) {
        for (auto& [partyId, campaign] : partyCampaigns) {
            campaign.popularity += campaign.momentum * dt * 0.001;
            campaign.momentum *= 0.99;
            if (campaign.spent < campaign.budget) {
                double dailySpend = campaign.budget.billions * 0.01;
                campaign.spent.billions += dailySpend;
                campaign.mediaCoverage += dailySpend * 0.1;
            }
            if (campaign.dirtyCampaigning > 0.3 && RandomEngine::instance().chance(0.01)) {
                campaign.popularity -= 0.02;
            }
        }
    }
}

void ElectoralSystem::generatePolls(const SimDate& date) {
    for (auto countryId : world_.getAllCountryIds()) {
        auto partyIds = world_.getPartiesOfCountry(countryId);
        if (partyIds.empty()) continue;
        if (!scheduledElections_.count(countryId)) continue;

        PollData poll;
        poll.date = date;
        poll.country = countryId;
        poll.sampleSize = RandomEngine::instance().randInt(800, 3000);
        poll.marginOfError = 3.5 / std::sqrt(poll.sampleSize / 1000.0);

        double total = 0;
        for (auto pid : partyIds) {
            double share = world_.getParty(pid).popularity + RandomEngine::instance().randDouble(-0.03, 0.03);
            share = std::max(0.01, share);
            poll.partySupport[pid] = share;
            total += share;
        }
        if (total > 0) for (auto& [pid, s] : poll.partySupport) s /= total;

        pollHistory_[countryId].push_back(poll);
        if (pollHistory_[countryId].size() > 52)
            pollHistory_[countryId].erase(pollHistory_[countryId].begin());
    }
}

void ElectoralSystem::updatePartyPopularity(Country& country, double dt) {
    for (auto pid : world_.getPartiesOfCountry(country.id)) {
        auto& party = world_.getParty(pid);
        if (party.inPower) {
            double delta = 0;
            if (country.gdpGrowthRate > 0.03) delta += 0.001;
            if (country.gdpGrowthRate < 0) delta -= 0.002;
            if (country.unemploymentRate > 0.1) delta -= 0.001;
            if (country.inflation > 0.1) delta -= 0.001;
            party.popularity += delta * dt;
            party.popularity = std::clamp(party.popularity, 0.05, 0.95);
        }
    }
}

void ElectoralSystem::processReferendum(Referendum& ref, const SimDate& date) {
    auto& country = world_.getCountry(ref.country);
    ref.turnout = 0.5 + country.democracy * 0.15 + RandomEngine::instance().randDouble(-0.05, 0.05);
    ref.turnout = std::clamp(ref.turnout, 0.2, 0.9);
    ref.yesVote = 0.5 + RandomEngine::instance().randDouble(-0.2, 0.2);
    ref.noVote = 1.0 - ref.yesVote;
    ref.completed = true;
}

// ===== Ações do Jogador =====

void ElectoralSystem::callElection(CountryID country, const std::string& type, const SimDate& date) {
    scheduledElections_[country] = date;
    electionTypes_[country] = type;
    world_.getCountry(country).internalStability -= 0.03;
}

void ElectoralSystem::setCampaignStrategy(CountryID country, PartyID party, const ElectionCampaign& campaign) {
    campaigns_[country][party] = campaign;
}

void ElectoralSystem::allocateCampaignFunds(CountryID country, PartyID party, Money amount) {
    campaigns_[country][party].budget = amount;
}

void ElectoralSystem::callReferendum(CountryID country, const std::string& question, const SimDate& date) {
    Referendum ref;
    ref.id = nextRefId_++;
    ref.country = country;
    ref.question = question;
    ref.date = date;
    referendums_.push_back(ref);
}

// ===== Consultas =====

bool ElectoralSystem::isElectionScheduled(CountryID country) const {
    return scheduledElections_.count(country) > 0;
}

SimDate ElectoralSystem::getNextElectionDate(CountryID country) const {
    auto it = scheduledElections_.find(country);
    if (it != scheduledElections_.end()) return it->second;
    return {};
}

std::vector<ElectionResult> ElectoralSystem::getElectionHistory(CountryID country) const {
    auto it = electionHistory_.find(country);
    if (it != electionHistory_.end()) return it->second;
    return {};
}

PollData ElectoralSystem::getLatestPoll(CountryID country) const {
    auto it = pollHistory_.find(country);
    if (it != pollHistory_.end() && !it->second.empty()) return it->second.back();
    return {};
}

std::vector<PollData> ElectoralSystem::getPollHistory(CountryID country) const {
    auto it = pollHistory_.find(country);
    if (it != pollHistory_.end()) return it->second;
    return {};
}

ElectionCampaign ElectoralSystem::getCampaign(CountryID country, PartyID party) const {
    auto cit = campaigns_.find(country);
    if (cit != campaigns_.end()) {
        auto pit = cit->second.find(party);
        if (pit != cit->second.end()) return pit->second;
    }
    return {};
}

double ElectoralSystem::getElectionFraudRisk(CountryID country) const {
    auto& c = world_.getCountry(country);
    return std::max(0.0, c.corruption - 0.3) + std::max(0.0, 0.5 - c.democracy);
}

double ElectoralSystem::predictElectionOutcome(CountryID country, PartyID party) const {
    return world_.getParty(party).popularity;
}

} // namespace GPS
