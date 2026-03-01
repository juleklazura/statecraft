/**
 * GPS - Geopolitical Simulator
 * PoliticsSystem.cpp - Implementação do sistema político
 */

#include "systems/politics/PoliticsSystem.h"
#include <iostream>
#include <cmath>

namespace GPS {

PoliticsSystem::PoliticsSystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void PoliticsSystem::init() {
    std::cout << "[Politics] Initializing political systems..." << std::endl;

    for (auto id : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(id);

        // Inicializar parlamento
        ParliamentState parliament;
        parliament.totalSeats = static_cast<int>(country.population * 0.5); // Simplificado
        if (parliament.totalSeats < 50) parliament.totalSeats = 50;
        if (parliament.totalSeats > 600) parliament.totalSeats = 600;

        auto partyIds = world_.getPartiesOfCountry(id);
        int seatsRemaining = parliament.totalSeats;
        for (size_t i = 0; i < partyIds.size(); i++) {
            auto& party = world_.getParty(partyIds[i]);
            int seats = static_cast<int>(party.popularity * parliament.totalSeats);
            if (i == partyIds.size() - 1) seats = seatsRemaining;
            seats = std::max(0, std::min(seats, seatsRemaining));
            parliament.seatDistribution[partyIds[i]] = seats;
            party.seatsParliament = seats;
            seatsRemaining -= seats;
        }

        // Calcular maioria do governo
        int govSeats = 0;
        for (auto pid : country.coalitionParties) {
            if (parliament.seatDistribution.count(pid)) {
                govSeats += parliament.seatDistribution[pid];
            }
        }
        if (parliament.seatDistribution.count(country.rulingPartyId)) {
            govSeats += parliament.seatDistribution[country.rulingPartyId];
        }
        parliament.governmentMajority = parliament.totalSeats > 0 ?
            static_cast<double>(govSeats) / parliament.totalSeats : 0.0;
        parliament.hasSupermajority = parliament.governmentMajority >= 0.67;

        parliaments_[id] = parliament;

        // Inicializar mídia
        MediaState media;
        media.pressLiberty = country.pressLiberty;
        media.governmentFavorability = country.governmentApproval;
        media.mediaTrust = 0.5;
        media_[id] = media;

        emergencyStates_[id] = false;
    }
}

void PoliticsSystem::update(double deltaTime, const SimDate& currentDate) {
    for (auto id : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(id);
        updatePoliticalStability(country, deltaTime);
        updateParliament(country, deltaTime);
        updateMedia(country, deltaTime);
        updateOpposition(country, deltaTime);
        checkForCrises(country, currentDate);
        resolveCrises(country, deltaTime);
        calculateGovernmentApproval(country);
    }
}

void PoliticsSystem::shutdown() {
    std::cout << "[Politics] Shutting down." << std::endl;
}

void PoliticsSystem::updatePoliticalStability(Country& country, double dt) {
    double stabilityFactors = 0.0;

    // Fatores positivos
    stabilityFactors += country.governmentApproval * 0.3;
    stabilityFactors += country.ruleOfLaw * 0.2;
    stabilityFactors += country.democracy * 0.1;
    stabilityFactors += (1.0 - country.corruption) * 0.15;

    // Fatores negativos
    stabilityFactors -= country.unemploymentRate * 0.2;
    stabilityFactors -= country.inflation * 0.5;
    stabilityFactors -= country.giniCoefficient * 0.1;

    // Social groups radicalisation
    auto groups = world_.getSocialGroupsOfCountry(country.id);
    double avgRadicalization = 0.0;
    for (auto gid : groups) {
        avgRadicalization += world_.getSocialGroup(gid).radicalization;
    }
    if (!groups.empty()) avgRadicalization /= groups.size();
    stabilityFactors -= avgRadicalization * 0.3;

    // Atualizar estabilidade interna
    double targetStability = std::clamp(stabilityFactors, 0.0, 1.0);
    double lerp = 0.01 * dt;
    country.internalStability += (targetStability - country.internalStability) * lerp;
    country.internalStability = std::clamp(country.internalStability, 0.0, 1.0);
}

void PoliticsSystem::updateParliament(Country& country, double dt) {
    auto& parliament = parliaments_[country.id];

    // Oposição se fortalece se governo é impopular
    parliament.oppositionStrength = std::clamp(
        1.0 - country.governmentApproval * 0.8 - parliament.governmentMajority * 0.2,
        0.0, 1.0
    );
}

void PoliticsSystem::updateMedia(Country& country, double dt) {
    auto& media = media_[country.id];

    // Mídia reflete e amplifica opinião pública
    double bias = (country.pressLiberty > 0.7) ? 0.0 : (0.7 - country.pressLiberty) * 0.5;
    media.governmentFavorability = country.governmentApproval * (1.0 + bias);
    media.governmentFavorability = std::clamp(media.governmentFavorability, 0.0, 1.0);

    // Jornalismo investigativo pode descobrir corrupção
    if (country.pressLiberty > 0.6 && media.investigativeJournalism > 0.5) {
        if (country.corruption > 0.5) {
            // Chance de escândalo
            if (RandomEngine::instance().chance(country.corruption * 0.001 * dt)) {
                media.recentHeadlines.push_back("Escândalo de corrupção revelado!");
                country.governmentApproval -= 0.05;
            }
        }
    }
}

void PoliticsSystem::updateOpposition(Country& country, double dt) {
    auto& parliament = parliaments_[country.id];

    // Oposição ganha força quando governo é fraco
    if (country.governmentApproval < 0.3 && parliament.governmentMajority < 0.5) {
        parliament.oppositionStrength += 0.001 * dt;
    }

    // Protestos podem surgir se satisfação geral é baixa
    auto groups = world_.getSocialGroupsOfCountry(country.id);
    for (auto gid : groups) {
        auto& group = world_.getSocialGroup(gid);
        if (group.satisfaction < 0.3 && group.radicalization > 0.5) {
            group.radicalization += 0.001 * dt * config_.publicOpinionSensitivity;
            group.radicalization = std::clamp(group.radicalization, 0.0, 1.0);
        }
    }
}

void PoliticsSystem::checkForCrises(Country& country, const SimDate& date) {
    auto& crisesList = crises_[country.id];

    // Verificar risco de golpe
    if (getCoupRisk(country.id) > 0.7) {
        if (RandomEngine::instance().chance(0.001)) {
            PoliticalCrisis crisis;
            crisis.type = PoliticalCrisis::Type::COUP_ATTEMPT;
            crisis.country = country.id;
            crisis.startDate = date;
            crisis.severity = getCoupRisk(country.id);
            crisis.description = "Tentativa de golpe militar!";
            crisesList.push_back(crisis);
        }
    }

    // Verificar impeachment
    if (getImpeachmentRisk(country.id) > 0.6 && country.democracy > 0.5) {
        if (RandomEngine::instance().chance(0.0005)) {
            PoliticalCrisis crisis;
            crisis.type = PoliticalCrisis::Type::IMPEACHMENT;
            crisis.country = country.id;
            crisis.startDate = date;
            crisis.severity = 0.7;
            crisis.description = "Processo de impeachment iniciado!";
            crisesList.push_back(crisis);
        }
    }

    // Protestos em massa
    double unrest = getRevolutionRisk(country.id);
    if (unrest > 0.5) {
        if (RandomEngine::instance().chance(unrest * 0.005)) {
            PoliticalCrisis crisis;
            crisis.type = PoliticalCrisis::Type::MASS_PROTESTS;
            crisis.country = country.id;
            crisis.startDate = date;
            crisis.severity = unrest;
            crisis.description = "Protestos massivos nas ruas!";
            crisesList.push_back(crisis);
        }
    }
}

void PoliticsSystem::resolveCrises(Country& country, double dt) {
    auto& crisesList = crises_[country.id];

    for (auto& crisis : crisesList) {
        if (crisis.resolved) continue;

        // Severidade muda com o tempo
        if (crisis.momentum > 0) {
            crisis.severity += crisis.momentum * dt * 0.01;
        } else {
            crisis.severity -= 0.005 * dt; // Decai naturalmente
        }

        crisis.severity = std::clamp(crisis.severity, 0.0, 1.0);

        // Resolver se severidade cair muito
        if (crisis.severity < 0.1) {
            crisis.resolved = true;
        }

        // Efeitos contínuos da crise
        country.governmentApproval -= crisis.severity * 0.01 * dt;
        country.internalStability -= crisis.severity * 0.005 * dt;
    }

    // Remover crises resolvidas mais antigas
    crisesList.erase(
        std::remove_if(crisesList.begin(), crisesList.end(),
                       [](const PoliticalCrisis& c) { return c.resolved; }),
        crisesList.end()
    );
}

void PoliticsSystem::calculateGovernmentApproval(Country& country) {
    // Approval é influenciado por muitos fatores
    double targetApproval = 0.5;

    // Economia
    targetApproval += (country.gdpGrowthRate - 0.02) * 2.0;
    targetApproval -= country.unemploymentRate * 0.5;
    targetApproval -= (country.inflation - 0.02) * 1.5;

    // Social
    auto groups = world_.getSocialGroupsOfCountry(country.id);
    double avgSatisfaction = 0.0;
    for (auto gid : groups) {
        avgSatisfaction += world_.getSocialGroup(gid).satisfaction;
    }
    if (!groups.empty()) avgSatisfaction /= groups.size();
    targetApproval += (avgSatisfaction - 0.5) * 0.3;

    // Governança
    targetApproval -= country.corruption * 0.2;
    targetApproval += country.ruleOfLaw * 0.1;

    targetApproval = std::clamp(targetApproval, 0.05, 0.95);

    // Lerp suave
    country.governmentApproval += (targetApproval - country.governmentApproval) * 0.005;
    country.governmentApproval = std::clamp(country.governmentApproval, 0.0, 1.0);
}

// ===== Ações do Jogador =====

bool PoliticsSystem::executeGovernmentAction(const GovernmentAction& action) {
    auto& country = world_.getCountry(action.country);
    auto& parliament = parliaments_[action.country];

    bool needsParliament = (action.type == GovernmentAction::Type::PROPOSE_LAW ||
                            action.type == GovernmentAction::Type::DISSOLVE_PARLIAMENT);

    if (needsParliament && parliament.governmentMajority < 0.5) {
        // Pode falhar sem maioria
        if (!RandomEngine::instance().chance(parliament.governmentMajority)) {
            return false;
        }
    }

    country.governmentApproval += action.popularityImpact;
    country.internalStability += action.stabilityImpact;

    actionHistory_[action.country].push_back(action);
    return true;
}

void PoliticsSystem::declareEmergency(CountryID country) {
    emergencyStates_[country] = true;
    auto& c = world_.getCountry(country);
    c.pressLiberty *= 0.7;
    c.democracy -= 0.1;
}

void PoliticsSystem::endEmergency(CountryID country) {
    emergencyStates_[country] = false;
}

void PoliticsSystem::reshuffleCabinet(CountryID country) {
    auto& c = world_.getCountry(country);
    c.governmentApproval += RandomEngine::instance().randDouble(-0.05, 0.05);
    c.institutionalTrust += 0.02;
}

void PoliticsSystem::adjustMediaControl(CountryID country, double level) {
    auto& c = world_.getCountry(country);
    c.pressLiberty = std::clamp(1.0 - level, 0.0, 1.0);
    media_[country].propagandaLevel = level;

    if (level > 0.5) {
        c.democracy -= level * 0.01;
    }
}

// ===== Consultas =====

ParliamentState PoliticsSystem::getParliamentState(CountryID country) const {
    auto it = parliaments_.find(country);
    if (it != parliaments_.end()) return it->second;
    return {};
}

MediaState PoliticsSystem::getMediaState(CountryID country) const {
    auto it = media_.find(country);
    if (it != media_.end()) return it->second;
    return {};
}

double PoliticsSystem::getGovernmentApproval(CountryID country) const {
    return world_.getCountry(country).governmentApproval;
}

double PoliticsSystem::getInstitutionalStability(CountryID country) const {
    return world_.getCountry(country).internalStability;
}

std::vector<PoliticalCrisis> PoliticsSystem::getActiveCrises(CountryID country) const {
    auto it = crises_.find(country);
    if (it != crises_.end()) return it->second;
    return {};
}

double PoliticsSystem::getCoupRisk(CountryID country) const {
    auto& c = world_.getCountry(country);
    double risk = 0.0;
    risk += (1.0 - c.democracy) * 0.3;
    risk += (1.0 - c.governmentApproval) * 0.2;
    risk += c.corruption * 0.2;
    risk += c.military.morale < 0.3 ? 0.2 : 0.0;
    risk += (1.0 - c.internalStability) * 0.1;
    return std::clamp(risk, 0.0, 1.0);
}

double PoliticsSystem::getImpeachmentRisk(CountryID country) const {
    auto& c = world_.getCountry(country);
    auto& parliament = parliaments_.at(country);
    double risk = 0.0;
    risk += (1.0 - c.governmentApproval) * 0.3;
    risk += parliament.oppositionStrength * 0.3;
    risk += c.corruption * 0.2;
    risk += (1.0 - parliament.governmentMajority) * 0.2;
    return std::clamp(risk, 0.0, 1.0);
}

double PoliticsSystem::getRevolutionRisk(CountryID country) const {
    auto& c = world_.getCountry(country);
    double risk = 0.0;
    risk += (1.0 - c.internalStability) * 0.3;
    risk += (1.0 - c.governmentApproval) * 0.2;
    risk += c.unemploymentRate * 0.2;
    risk += c.giniCoefficient * 0.15;
    risk += (c.inflation > 0.1 ? (c.inflation - 0.1) * 2.0 : 0.0) * 0.15;
    return std::clamp(risk, 0.0, 1.0);
}

bool PoliticsSystem::isInEmergency(CountryID country) const {
    auto it = emergencyStates_.find(country);
    return it != emergencyStates_.end() && it->second;
}

bool PoliticsSystem::proposeLaw(CountryID country, LawID lawId) {
    // Delegado ao LawSystem
    return true;
}

bool PoliticsSystem::revokeLaw(CountryID country, LawID lawId) {
    return true;
}

void PoliticsSystem::callReferendum(CountryID country, const std::string& question) {
    // Delega ao ElectoralSystem
}

} // namespace GPS
