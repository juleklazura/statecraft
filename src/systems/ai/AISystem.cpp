/**
 * GPS - Geopolitical Simulator
 * AISystem.cpp - Matches AISystem.h exactly
 */

#include "systems/ai/AISystem.h"
#include "systems/economy/EconomySystem.h"
#include "systems/politics/PoliticsSystem.h"
#include "systems/diplomacy/DiplomacySystem.h"
#include "systems/military/MilitarySystem.h"
#include "systems/laws/LawSystem.h"
#include <iostream>
#include <algorithm>

namespace GPS {

AISystem::AISystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void AISystem::init() {
    std::cout << "[AI] Initializing AI decision system..." << std::endl;

    for (auto id : world_.getAllCountryIds()) {
        if (id == world_.getPlayerCountry()) continue;
        // Gerar personalidade baseada no tipo de governo
        AIPersonality p;
        auto& country = world_.getCountry(id);
        switch (country.government) {
            case GovernmentType::PRESIDENTIAL_REPUBLIC:
            case GovernmentType::PARLIAMENTARY_REPUBLIC:
            case GovernmentType::CONSTITUTIONAL_MONARCHY:
                p.aggression = RandomEngine::instance().randDouble(0.1, 0.4);
                p.diplomacy = RandomEngine::instance().randDouble(0.5, 0.9);
                p.economicFocus = RandomEngine::instance().randDouble(0.5, 0.8);
                p.militarism = RandomEngine::instance().randDouble(0.1, 0.4);
                p.riskTolerance = RandomEngine::instance().randDouble(0.2, 0.5);
                p.pragmatism = RandomEngine::instance().randDouble(0.5, 0.8);
                p.authoritarianism = RandomEngine::instance().randDouble(0.1, 0.3);
                p.priorityEconomy = 0.30; p.priorityDiplomacy = 0.25;
                p.priorityMilitary = 0.15; p.priorityInternal = 0.20; p.priorityScience = 0.10;
                break;
            case GovernmentType::ONE_PARTY_STATE:
            case GovernmentType::MILITARY_JUNTA:
                p.aggression = RandomEngine::instance().randDouble(0.3, 0.7);
                p.diplomacy = RandomEngine::instance().randDouble(0.2, 0.5);
                p.militarism = RandomEngine::instance().randDouble(0.5, 0.9);
                p.riskTolerance = RandomEngine::instance().randDouble(0.4, 0.8);
                p.authoritarianism = RandomEngine::instance().randDouble(0.6, 0.9);
                p.priorityMilitary = 0.35; p.priorityInternal = 0.25;
                p.priorityEconomy = 0.20; p.priorityDiplomacy = 0.10; p.priorityScience = 0.10;
                break;
            default:
                p.aggression = RandomEngine::instance().randDouble(0.2, 0.5);
                p.diplomacy = RandomEngine::instance().randDouble(0.3, 0.7);
                p.economicFocus = RandomEngine::instance().randDouble(0.3, 0.7);
                p.militarism = RandomEngine::instance().randDouble(0.2, 0.5);
                p.riskTolerance = RandomEngine::instance().randDouble(0.3, 0.5);
                break;
        }
        personalities_[id] = p;
    }
}

void AISystem::update(double deltaTime, const SimDate& currentDate) {
    if (currentDate.hour != 0) return;
    if (currentDate.day % 3 != 0) return; // Decisões a cada 3 dias

    for (auto countryId : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(countryId);
        if (countryId == world_.getPlayerCountry()) continue;
        if (!country.isAIActive) continue;

        auto it = personalities_.find(countryId);
        if (it == personalities_.end()) continue;

        updateCountryAI(country, currentDate, deltaTime);
    }
}

void AISystem::shutdown() {
    std::cout << "[AI] Shutting down." << std::endl;
}

void AISystem::updateCountryAI(Country& country, const SimDate& date, double dt) {
    auto& personality = personalities_[country.id];

    evaluateThreats(country);
    evaluateOpportunities(country);

    makeEconomicDecisions(country, personality);
    makeMilitaryDecisions(country, personality);
    makeDiplomaticDecisions(country, personality);
    makeInternalDecisions(country, personality);
    respondToCrisis(country, personality);
}

void AISystem::evaluateThreats(Country& country) {
    // Avaliar ameaças econômicas e militares
    // (informação usada internamente pelos métodos de decisão)
}

void AISystem::evaluateOpportunities(Country& country) {
    // Avaliar oportunidades diplomáticas e comerciais
}

void AISystem::makeEconomicDecisions(Country& country, const AIPersonality& personality) {
    if (!economySystem_) return;

    if (country.inflation > 0.1) {
        double newRate = country.interestRate + 0.005;
        economySystem_->setInterestRate(country.id, std::min(newRate, 0.2));
    } else if (country.gdpGrowthRate < -0.02) {
        double newRate = country.interestRate - 0.005;
        economySystem_->setInterestRate(country.id, std::max(newRate, 0.001));
    }
}

void AISystem::makeMilitaryDecisions(Country& country, const AIPersonality& personality) {
    if (!militarySystem_) return;

    // Aumentar defesa se ameaçado
    for (auto otherId : world_.getAllCountryIds()) {
        if (otherId == country.id) continue;
        auto rel = world_.getRelation(country.id, otherId);
        if (rel.relations < -50) {
            country.budgetDefense = std::min(country.budgetDefense + 0.01, 0.15);
            break;
        }
    }

    // Países agressivos podem declarar guerra
    if (personality.aggression > 0.7 && personality.riskTolerance > 0.6 && diplomacySystem_) {
        for (auto otherId : world_.getAllCountryIds()) {
            if (otherId == country.id) continue;
            auto rel = world_.getRelation(country.id, otherId);
            auto& other = world_.getCountry(otherId);
            if (rel.relations < -70 && rel.status != DiplomaticStatus::AT_WAR &&
                assessMilitaryBalance(country.id, otherId) > 2.0) {
                if (RandomEngine::instance().chance(personality.aggression * 0.005)) {
                    diplomacySystem_->declareWar(country.id, otherId, "Conflito territorial");
                }
            }
        }
    }
}

void AISystem::makeDiplomaticDecisions(Country& country, const AIPersonality& personality) {
    if (!diplomacySystem_) return;
    if (personality.cooperation < 0.3) return;

    for (auto otherId : world_.getAllCountryIds()) {
        if (otherId == country.id) continue;
        auto rel = world_.getRelation(country.id, otherId);
        if (rel.relations > 30 && !rel.hasTradeAgreement && RandomEngine::instance().chance(0.005)) {
            DiplomaticProposal proposal;
            proposal.type = DiplomaticProposal::Type::TRADE_AGREEMENT;
            proposal.proposer = country.id;
            proposal.target = otherId;
            diplomacySystem_->sendProposal(proposal);
        }
    }
}

void AISystem::makeInternalDecisions(Country& country, const AIPersonality& personality) {
    if (!politicsSystem_) return;

    if (country.internalStability < 0.3 && personality.authoritarianism > 0.6) {
        politicsSystem_->declareEmergency(country.id);
    }
}

void AISystem::respondToCrisis(Country& country, const AIPersonality& personality) {
    // Respostas automáticas a crises (economia, instabilidade)
    if (country.gdpGrowthRate < -0.05 && economySystem_) {
        economySystem_->setInterestRate(country.id, std::max(country.interestRate - 0.01, 0.001));
    }
}

double AISystem::assessCountryRisk(CountryID country) const {
    auto& c = world_.getCountry(country);
    double risk = 0;
    risk += std::max(0.0, -c.gdpGrowthRate) * 5.0;
    risk += std::max(0.0, c.inflation - 0.05) * 3.0;
    risk += std::max(0.0, 0.7 - c.internalStability) * 2.0;
    risk += std::max(0.0, c.debtToGDP - 0.6) * 2.0;
    return std::clamp(risk, 0.0, 1.0);
}

double AISystem::assessMilitaryBalance(CountryID us, CountryID them) const {
    auto& our = world_.getCountry(us);
    auto& their = world_.getCountry(them);
    double ourPower = our.military.calculatePowerIndex();
    double theirPower = their.military.calculatePowerIndex();
    if (theirPower < 0.001) return 100.0;
    return ourPower / theirPower;
}

// ===== Interface =====

void AISystem::linkSystems(EconomySystem* eco, PoliticsSystem* pol,
                           DiplomacySystem* dip, MilitarySystem* mil,
                           LawSystem* law) {
    economySystem_ = eco;
    politicsSystem_ = pol;
    diplomacySystem_ = dip;
    militarySystem_ = mil;
    lawSystem_ = law;
}

void AISystem::setPersonality(CountryID country, const AIPersonality& personality) {
    personalities_[country] = personality;
}

AIPersonality AISystem::getPersonality(CountryID country) const {
    auto it = personalities_.find(country);
    if (it != personalities_.end()) return it->second;
    return {};
}

void AISystem::setAIActive(CountryID country, bool active) {
    world_.getCountry(country).isAIActive = active;
}

bool AISystem::isAIActive(CountryID country) const {
    return world_.getCountry(country).isAIActive;
}

std::vector<AIDecision> AISystem::getRecentDecisions(CountryID country, int maxDays) const {
    auto it = decisionHistory_.find(country);
    if (it != decisionHistory_.end()) return it->second;
    return {};
}

} // namespace GPS
