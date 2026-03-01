/**
 * GPS - Geopolitical Simulator
 * IntelligenceSystem.cpp
 */

#include "systems/intelligence/IntelligenceSystem.h"
#include <iostream>

namespace GPS {

IntelligenceSystem::IntelligenceSystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void IntelligenceSystem::init() {
    std::cout << "[Intelligence] Initializing intelligence agencies..." << std::endl;
    for (auto id : world_.getAllCountryIds()) {
        IntelligenceAgency agency;
        agency.country = id;
        auto& c = world_.getCountry(id);
        agency.budget = c.gdp.billions * 0.005; // ~0.5% do PIB
        agency.capability = c.hdi * 0.5 + c.military.technology * 0.3;
        agency.counterIntel = c.cyberDefense;
        agency.cyberCapability = c.cyberDefense;
        agency.name = c.name + " Intelligence Service";
        agencies_[id] = agency;
    }
}

void IntelligenceSystem::update(double deltaTime, const SimDate& currentDate) {
    if (!config_.enableEspionage) return;
    updateOperations(deltaTime, currentDate);
    if (currentDate.hour == 6) {
        generateIntelReports(currentDate);
    }
}

void IntelligenceSystem::shutdown() {
    std::cout << "[Intelligence] Shutting down." << std::endl;
}

void IntelligenceSystem::updateOperations(double dt, const SimDate& date) {
    for (auto& [id, op] : operations_) {
        if (op.completed) continue;

        op.daysElapsed += static_cast<int>(dt);

        // Verificar descoberta
        checkForDiscovery(op);

        // Completar operação
        if (op.daysElapsed >= op.daysToComplete) {
            resolveOperation(op);
        }
    }
}

void IntelligenceSystem::checkForDiscovery(CovertOperation& op) {
    auto& targetAgency = agencies_[op.target];
    double discoveryChance = op.discoveryRisk * targetAgency.counterIntel;
    if (RandomEngine::instance().chance(discoveryChance * 0.01)) {
        op.discovered = true;
        handleDiscovery(op);
    }
}

void IntelligenceSystem::resolveOperation(CovertOperation& op) {
    op.completed = true;
    auto& agency = agencies_[op.operator_];

    if (RandomEngine::instance().chance(op.successProbability * agency.capability)) {
        op.successful = true;
        applyOperationEffects(op);
    }
}

void IntelligenceSystem::applyOperationEffects(const CovertOperation& op) {
    auto& target = world_.getCountry(op.target);

    switch (op.type) {
        case CovertOperation::Type::SABOTAGE:
            target.gdp.billions *= 0.99;
            target.internalStability -= 0.05;
            break;
        case CovertOperation::Type::PROPAGANDA:
            target.governmentApproval -= 0.05;
            break;
        case CovertOperation::Type::ELECTION_INTERFERENCE:
            target.governmentApproval -= 0.1;
            target.democracy -= 0.05;
            break;
        case CovertOperation::Type::CYBER_ATTACK:
            target.cyberDefense -= 0.1;
            target.internalStability -= 0.03;
            break;
        case CovertOperation::Type::COUP_SUPPORT:
            target.internalStability -= 0.15;
            target.governmentApproval -= 0.1;
            break;
        case CovertOperation::Type::ECONOMIC_WARFARE:
            target.gdp.billions *= 0.98;
            target.inflation += 0.02;
            break;
        default:
            break;
    }
}

void IntelligenceSystem::handleDiscovery(const CovertOperation& op) {
    auto& rel = world_.getRelation(op.operator_, op.target);
    rel.relations -= 0.15;
    rel.trust -= 0.2;

    // Impacto diplomático global
    for (auto id : world_.getAllCountryIds()) {
        if (id == op.operator_ || id == op.target) continue;
        auto& r = world_.getRelation(op.operator_, id);
        r.trust -= 0.02;
    }

    // Gerar relatório
    IntelReport report;
    report.about = op.operator_;
    report.content = "Operação de espionagem descoberta: " + op.codeName;
    report.category = IntelReport::Category::ESPIONAGE_DETECTED;
    report.reliability = 1.0;
    reports_[op.target].push_back(report);
}

void IntelligenceSystem::generateIntelReports(const SimDate& date) {
    for (auto id : world_.getAllCountryIds()) {
        auto& agency = agencies_[id];
        if (agency.capability < 0.3) continue;

        // Gerar intel sobre países vizinhos/rivais
        auto& country = world_.getCountry(id);
        for (auto neighbor : country.neighbors) {
            if (RandomEngine::instance().chance(agency.capability * 0.05)) {
                auto& target = world_.getCountry(neighbor);
                IntelReport report;
                report.about = neighbor;
                report.date = date;
                report.reliability = agency.capability * 0.8;

                if (target.military.activePersonnel > country.military.activePersonnel * 1.5) {
                    report.category = IntelReport::Category::MILITARY_MOVEMENT;
                    report.content = target.name + " tem mobilização militar significativa.";
                } else if (target.internalStability < 0.3) {
                    report.category = IntelReport::Category::POLITICAL_INSTABILITY;
                    report.content = target.name + " enfrenta instabilidade política séria.";
                } else {
                    report.category = IntelReport::Category::ECONOMIC_DATA;
                    report.content = target.name + ": PIB growth " +
                                    std::to_string(target.gdpGrowthRate * 100) + "%";
                }

                reports_[id].push_back(report);
            }
        }
    }
}

// ===== Ações do Jogador =====

OperationID IntelligenceSystem::launchOperation(CountryID country, CovertOperation::Type type,
                                                  CountryID target, const std::string& codeName) {
    auto& agency = agencies_[country];
    CovertOperation op;
    op.id = nextOpId_++;
    op.operator_ = country;
    op.target = target;
    op.type = type;
    op.codeName = codeName;

    // Configurar baseado no tipo
    switch (type) {
        case CovertOperation::Type::ESPIONAGE:
            op.daysToComplete = 60; op.successProbability = 0.6;
            op.discoveryRisk = 0.2; break;
        case CovertOperation::Type::SABOTAGE:
            op.daysToComplete = 30; op.successProbability = 0.4;
            op.discoveryRisk = 0.4; break;
        case CovertOperation::Type::CYBER_ATTACK:
            op.daysToComplete = 14; op.successProbability = 0.5;
            op.discoveryRisk = 0.3; break;
        case CovertOperation::Type::PROPAGANDA:
            op.daysToComplete = 90; op.successProbability = 0.7;
            op.discoveryRisk = 0.1; break;
        case CovertOperation::Type::ELECTION_INTERFERENCE:
            op.daysToComplete = 180; op.successProbability = 0.3;
            op.discoveryRisk = 0.5; break;
        case CovertOperation::Type::COUP_SUPPORT:
            op.daysToComplete = 365; op.successProbability = 0.2;
            op.discoveryRisk = 0.6; break;
        default:
            op.daysToComplete = 30; op.successProbability = 0.5;
            op.discoveryRisk = 0.3; break;
    }

    // Modificar com capacidade da agência
    op.successProbability *= agency.capability;
    op.discoveryRisk *= (1.0 - agency.secrecy);

    operations_[op.id] = op;
    return op.id;
}

void IntelligenceSystem::cancelOperation(OperationID id) {
    auto it = operations_.find(id);
    if (it != operations_.end()) {
        it->second.completed = true;
    }
}

void IntelligenceSystem::setIntelBudget(CountryID country, double budgetBillions) {
    agencies_[country].budget = budgetBillions;
    agencies_[country].capability = std::clamp(budgetBillions * 10.0, 0.0, 1.0);
}

void IntelligenceSystem::increaseCounterIntelligence(CountryID country) {
    agencies_[country].counterIntel = std::min(1.0, agencies_[country].counterIntel + 0.05);
}

void IntelligenceSystem::recruitOperatives(CountryID country, int count) {
    agencies_[country].operativeCount += count;
    agencies_[country].humanIntel = std::min(1.0, agencies_[country].operativeCount / 10000.0);
}

// ===== Consultas =====

IntelligenceAgency IntelligenceSystem::getAgency(CountryID country) const {
    auto it = agencies_.find(country);
    if (it != agencies_.end()) return it->second;
    return {};
}

std::vector<CovertOperation> IntelligenceSystem::getActiveOperations(CountryID country) const {
    std::vector<CovertOperation> result;
    for (const auto& [id, op] : operations_) {
        if (op.operator_ == country && !op.completed) result.push_back(op);
    }
    return result;
}

std::vector<IntelReport> IntelligenceSystem::getRecentReports(CountryID country, int maxDays) const {
    auto it = reports_.find(country);
    if (it != reports_.end()) return it->second;
    return {};
}

double IntelligenceSystem::getIntelligenceRating(CountryID country) const {
    auto it = agencies_.find(country);
    return it != agencies_.end() ? it->second.capability : 0.0;
}

std::vector<CovertOperation> IntelligenceSystem::getDiscoveredOperationsAgainst(CountryID country) const {
    std::vector<CovertOperation> result;
    for (const auto& [id, op] : operations_) {
        if (op.target == country && op.discovered) result.push_back(op);
    }
    return result;
}

bool IntelligenceSystem::hasIntelOn(CountryID observer, CountryID target) const {
    auto it = reports_.find(observer);
    if (it == reports_.end()) return false;
    for (const auto& report : it->second) {
        if (report.about == target) return true;
    }
    return false;
}

} // namespace GPS
