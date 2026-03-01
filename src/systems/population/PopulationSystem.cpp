/**
 * GPS - Geopolitical Simulator
 * PopulationSystem.cpp - Implementação do sistema de população
 */

#include "systems/population/PopulationSystem.h"
#include <iostream>
#include <cmath>

namespace GPS {

PopulationSystem::PopulationSystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void PopulationSystem::init() {
    std::cout << "[Population] Initializing population simulation..." << std::endl;

    for (auto id : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(id);

        DemographicData demo;
        demo.totalPopulation = country.population;
        demo.birthRate = country.birthRate;
        demo.deathRate = country.deathRate;

        // Pirâmide etária baseada no IDH
        if (country.hdi > 0.8) {
            demo.age0to14 = 0.15; demo.age15to24 = 0.12;
            demo.age25to54 = 0.40; demo.age55to64 = 0.15;
            demo.age65plus = 0.18;
        } else if (country.hdi > 0.6) {
            demo.age0to14 = 0.25; demo.age15to24 = 0.17;
            demo.age25to54 = 0.38; demo.age55to64 = 0.12;
            demo.age65plus = 0.08;
        } else {
            demo.age0to14 = 0.38; demo.age15to24 = 0.20;
            demo.age25to54 = 0.30; demo.age55to64 = 0.07;
            demo.age65plus = 0.05;
        }

        demo.workingAgeShare = demo.age15to24 + demo.age25to54 + demo.age55to64;
        demo.dependencyRatio = (demo.age0to14 + demo.age65plus) / demo.workingAgeShare;
        demographics_[id] = demo;

        PublicOpinion opinion;
        opinion.governmentApproval = country.governmentApproval;
        opinion.economicOptimism = 0.5;
        opinion.securityPerception = country.internalStability;
        opinion.overallHappiness = country.hdi * 0.7 + 0.15;
        opinions_[id] = opinion;
    }
}

void PopulationSystem::update(double deltaTime, const SimDate& currentDate) {
    for (auto id : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(id);
        updateDemographics(country, deltaTime);
        updatePublicOpinion(country, deltaTime);
        updateSocialGroups(country, deltaTime);
        updateSocialMovements(country, deltaTime);
        checkForUnrest(country);
    }
}

void PopulationSystem::shutdown() {
    std::cout << "[Population] Shutting down." << std::endl;
}

void PopulationSystem::updateDemographics(Country& country, double dt) {
    auto& demo = demographics_[country.id];

    // Crescimento populacional
    double births = country.population * demo.birthRate * dt / 365.0;
    double deaths = country.population * demo.deathRate * dt / 365.0;
    double netMigration = country.population *
        (demo.immigrationRate - demo.emigrationRate) * dt / 365.0;

    country.population += (births - deaths + netMigration);
    if (country.population < 0.001) country.population = 0.001;

    // Atualizar expectativa de vida baseada em saúde/educação
    double healthFactor = country.budgetHealthcare * 2.0;
    double wealthFactor = std::log10(std::max(1.0, country.gdpPerCapita / 1000.0)) * 5.0;
    country.lifeExpectancy += (healthFactor + wealthFactor - country.lifeExpectancy * 0.01) * dt / 365.0 * 0.001;
    country.lifeExpectancy = std::clamp(country.lifeExpectancy, 40.0, 95.0);

    // Urbanização
    if (country.gdpGrowthRate > 0 && country.urbanizationRate < 0.95) {
        country.urbanizationRate += 0.001 * dt / 365.0;
    }

    // Atualizar dados demográficos
    demo.totalPopulation = country.population;
    demo.workingAgeShare = demo.age15to24 + demo.age25to54 + demo.age55to64;
    demo.dependencyRatio = (demo.age0to14 + demo.age65plus) /
                           std::max(0.01, demo.workingAgeShare);

    // Envelhecimento gradual em países desenvolvidos
    if (country.hdi > 0.7) {
        demo.age0to14 -= 0.0001 * dt / 365.0;
        demo.age65plus += 0.0001 * dt / 365.0;
    }
}

void PopulationSystem::updatePublicOpinion(Country& country, double dt) {
    auto& opinion = opinions_[country.id];

    // Otimismo econômico
    double econTarget = 0.5 + country.gdpGrowthRate * 5.0 -
                         country.unemploymentRate * 2.0 -
                         (country.inflation - 0.02) * 5.0;
    econTarget = std::clamp(econTarget, 0.0, 1.0);
    opinion.economicOptimism += (econTarget - opinion.economicOptimism) * 0.01 * dt;

    // Percepção de segurança
    double secTarget = country.internalStability * 0.5 +
                       (1.0 - country.terrorismThreat) * 0.3 +
                       country.borderSecurity * 0.2;
    opinion.securityPerception += (secTarget - opinion.securityPerception) * 0.01 * dt;

    // Confiança institucional
    double trustTarget = country.ruleOfLaw * 0.4 +
                         (1.0 - country.corruption) * 0.3 +
                         country.democracy * 0.3;
    opinion.trustInInstitutions += (trustTarget - opinion.trustInInstitutions) * 0.005 * dt;

    // Coesão social
    double cohesionTarget = 1.0 - country.giniCoefficient * 0.5 -
                            country.unemploymentRate * 0.3;
    opinion.socialCohesion += (cohesionTarget - opinion.socialCohesion) * 0.005 * dt;

    // Felicidade geral
    opinion.overallHappiness = (opinion.economicOptimism * 0.3 +
                                opinion.securityPerception * 0.2 +
                                opinion.trustInInstitutions * 0.2 +
                                opinion.socialCohesion * 0.15 +
                                (country.hdi - 0.5) * 0.15);
    opinion.overallHappiness = std::clamp(opinion.overallHappiness, 0.0, 1.0);

    // Approval do governo
    opinion.governmentApproval = country.governmentApproval;
    opinion.approvalTrend = country.gdpGrowthRate > 0.03 ? 0.01 : -0.01;
}

void PopulationSystem::updateSocialGroups(Country& country, double dt) {
    auto groupIds = world_.getSocialGroupsOfCountry(country.id);

    for (auto gid : groupIds) {
        auto& group = world_.getSocialGroup(gid);
        calculateGroupSatisfaction(country, group);

        // Radicalização aumenta quando satisfação é baixa
        if (group.satisfaction < 0.3) {
            group.radicalization += (0.3 - group.satisfaction) * 0.005 * dt *
                                   config_.publicOpinionSensitivity;
        } else if (group.satisfaction > 0.6) {
            group.radicalization -= 0.002 * dt;
        }
        group.radicalization = std::clamp(group.radicalization, 0.0, 1.0);

        // Apoio ao governo
        group.governmentSupport = group.satisfaction * 0.6 +
                                  (1.0 - group.radicalization) * 0.4;

        // Decair modificadores
        for (auto& mod : group.modifiers) {
            mod.decay();
        }
        group.modifiers.erase(
            std::remove_if(group.modifiers.begin(), group.modifiers.end(),
                          [](const Modifier& m) { return m.expired(); }),
            group.modifiers.end()
        );
    }
}

void PopulationSystem::calculateGroupSatisfaction(Country& country, SocialGroup& group) {
    double satisfaction = 0.5;

    // Emprego
    satisfaction += (1.0 - country.unemploymentRate) * group.interestEmployment * 0.3;

    // Renda (GDP per capita como proxy)
    double incomeScore = std::min(1.0, country.gdpPerCapita / 50000.0);
    satisfaction += incomeScore * group.interestWages * 0.2;

    // Impostos
    double taxBurden = country.taxBurden;
    if (group.type == SocialGroupType::UPPER_CLASS ||
        group.type == SocialGroupType::ENTREPRENEURS) {
        taxBurden = country.incomeTaxHigh; // Ricos sentem mais impostos altos
    }
    satisfaction -= taxBurden * group.interestTaxes * 0.15;

    // Segurança
    satisfaction += country.internalStability * group.interestSecurity * 0.1;

    // Serviços públicos
    satisfaction += country.budgetHealthcare * group.interestHealthcare * 0.1;
    satisfaction += country.budgetEducation * group.interestEducation * 0.1;

    // Desigualdade afeta mais os mais pobres
    if (group.type == SocialGroupType::LOWER_CLASS ||
        group.type == SocialGroupType::WORKERS) {
        satisfaction -= country.giniCoefficient * 0.2;
    }

    // Modificadores ativos
    for (const auto& mod : group.modifiers) {
        satisfaction += mod.magnitude;
    }

    group.satisfaction = std::clamp(satisfaction, 0.0, 1.0);
}

void PopulationSystem::updateSocialMovements(Country& country, double dt) {
    auto& movementList = movements_[country.id];

    for (auto& movement : movementList) {
        // Atualizar suporte
        if (movement.momentum > 0) {
            movement.support += movement.momentum * 0.01 * dt;
        } else {
            movement.support -= 0.005 * dt;
        }
        movement.support = std::clamp(movement.support, 0.0, 0.5); // Max 50% da pop

        // Radicalização do movimento
        if (country.governmentApproval < 0.3 && movement.support > 0.1) {
            movement.radicalization += 0.002 * dt;
        }

        // Movimentos violentos geram instabilidade
        if (movement.isViolent) {
            country.internalStability -= movement.support * 0.01 * dt;
        }
    }

    // Remover movimentos sem suporte
    movementList.erase(
        std::remove_if(movementList.begin(), movementList.end(),
                       [](const SocialMovement& m) { return m.support < 0.01; }),
        movementList.end()
    );
}

void PopulationSystem::checkForUnrest(Country& country) {
    auto& opinion = opinions_[country.id];

    // Se satisfação geral está muito baixa, possibilidade de movimento
    if (opinion.overallHappiness < 0.3 && RandomEngine::instance().chance(0.001)) {
        SocialMovement movement;
        movement.name = "Movimento de Protesto Popular";
        movement.country = country.id;
        movement.mainGroup = SocialGroupType::WORKERS;
        movement.support = 0.05;
        movement.momentum = 0.1;
        movement.type = SocialMovement::Type::PROTEST;
        movement.cause = "Insatisfação geral com o governo";
        movements_[country.id].push_back(movement);
    }
}

void PopulationSystem::processImmigration(Country& country, double dt) {
    auto& demo = demographics_[country.id];

    // Imigração é influenciada pela atratividade do país
    double attractiveness = country.gdpPerCapita / 50000.0 * 0.3 +
                           country.internalStability * 0.3 +
                           country.democracy * 0.2 +
                           (1.0 - country.unemploymentRate) * 0.2;

    demo.immigrationRate = attractiveness * 0.005;
    demo.emigrationRate = (1.0 - attractiveness) * 0.003;
}

// ===== Consultas =====

DemographicData PopulationSystem::getDemographics(CountryID country) const {
    auto it = demographics_.find(country);
    if (it != demographics_.end()) return it->second;
    return {};
}

PublicOpinion PopulationSystem::getPublicOpinion(CountryID country) const {
    auto it = opinions_.find(country);
    if (it != opinions_.end()) return it->second;
    return {};
}

std::vector<SocialMovement> PopulationSystem::getActiveMovements(CountryID country) const {
    auto it = movements_.find(country);
    if (it != movements_.end()) return it->second;
    return {};
}

double PopulationSystem::getGroupSatisfaction(CountryID country, SocialGroupType group) const {
    auto groups = world_.getSocialGroupsOfCountry(country);
    for (auto gid : groups) {
        auto& g = world_.getSocialGroup(gid);
        if (g.type == group) return g.satisfaction;
    }
    return 0.5;
}

double PopulationSystem::getGroupRadicalization(CountryID country, SocialGroupType group) const {
    auto groups = world_.getSocialGroupsOfCountry(country);
    for (auto gid : groups) {
        auto& g = world_.getSocialGroup(gid);
        if (g.type == group) return g.radicalization;
    }
    return 0.0;
}

double PopulationSystem::getRevoltRisk(CountryID country) const {
    auto& opinion = opinions_.at(country);
    double risk = (1.0 - opinion.overallHappiness) * 0.4 +
                  (1.0 - opinion.trustInInstitutions) * 0.3 +
                  (1.0 - opinion.socialCohesion) * 0.3;
    return std::clamp(risk, 0.0, 1.0);
}

double PopulationSystem::getUrbanRuralTension(CountryID country) const {
    auto& c = world_.getCountry(country);
    return std::clamp(std::abs(c.urbanizationRate - 0.5) * 0.5, 0.0, 1.0);
}

double PopulationSystem::getEthnicTension(CountryID country) const {
    return 0.2; // Simplificado
}

double PopulationSystem::getReligiousTension(CountryID country) const {
    return 0.1; // Simplificado
}

void PopulationSystem::applyPolicyImpact(CountryID country, SocialGroupType group,
                                          double satisfactionDelta, double radicalizationDelta) {
    auto groups = world_.getSocialGroupsOfCountry(country);
    for (auto gid : groups) {
        auto& g = world_.getSocialGroup(gid);
        if (g.type == group) {
            g.satisfaction = std::clamp(g.satisfaction + satisfactionDelta, 0.0, 1.0);
            g.radicalization = std::clamp(g.radicalization + radicalizationDelta, 0.0, 1.0);
            break;
        }
    }
}

void PopulationSystem::applyEconomicShock(CountryID country, double magnitude) {
    auto& opinion = opinions_[country];
    opinion.economicOptimism -= magnitude * 0.3;
    opinion.economicOptimism = std::clamp(opinion.economicOptimism, 0.0, 1.0);

    auto groups = world_.getSocialGroupsOfCountry(country);
    for (auto gid : groups) {
        auto& g = world_.getSocialGroup(gid);
        g.satisfaction -= magnitude * 0.1;
        g.satisfaction = std::clamp(g.satisfaction, 0.0, 1.0);
    }
}

void PopulationSystem::triggerSocialMovement(CountryID country, SocialMovement movement) {
    movements_[country].push_back(std::move(movement));
}

} // namespace GPS
