/**
 * GPS - Geopolitical Simulator
 * MilitarySystem.cpp - Implementação do sistema militar
 */

#include "systems/military/MilitarySystem.h"
#include <iostream>
#include <cmath>

namespace GPS {

MilitarySystem::MilitarySystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void MilitarySystem::init() {
    std::cout << "[Military] Initializing military simulation..." << std::endl;
}

void MilitarySystem::update(double deltaTime, const SimDate& currentDate) {
    updateUnits(deltaTime);
    updateConflicts(deltaTime, currentDate);
    updateOperations(deltaTime);

    // Atualizar orçamento e manutenção semanalmente
    if (currentDate.day % 7 == 0 && currentDate.hour == 0) {
        for (auto id : world_.getAllCountryIds()) {
            auto& country = world_.getCountry(id);
            country.military.annualBudget = Money(
                country.gdp.billions * country.military.gdpShare
            );
        }
    }
}

void MilitarySystem::shutdown() {
    std::cout << "[Military] Shutting down." << std::endl;
}

void MilitarySystem::updateConflicts(double dt, const SimDate& date) {
    for (auto& conflict : conflicts_) {
        if (!conflict.active) continue;

        processAttrition(conflict, dt);
        calculateBattleOutcome(conflict);
        checkEscalation(conflict);

        // Custo econômico da guerra
        for (auto attId : conflict.attackers) {
            auto& att = world_.getCountry(attId);
            double dailyCost = att.military.annualBudget.billions * 0.005 * dt;
            att.foreignReserves -= Money(dailyCost);
            if (att.foreignReserves.billions < 0.0) att.foreignReserves = Money(0.0);
            conflict.totalCost += Money(dailyCost);
            warWeariness_[attId] += 0.001 * dt;
        }
        for (auto defId : conflict.defenders) {
            auto& def = world_.getCountry(defId);
            double dailyCost = def.military.annualBudget.billions * 0.008 * dt;
            def.foreignReserves -= Money(dailyCost);
            if (def.foreignReserves.billions < 0.0) def.foreignReserves = Money(0.0);
            conflict.totalCost += Money(dailyCost);
            warWeariness_[defId] += 0.0005 * dt; // Defensor tem menos war weariness
        }
    }
}

void MilitarySystem::processAttrition(MilitaryConflict& conflict, double dt) {
    // Calcular poder total de cada lado
    double attackerPower = 0.0;
    double defenderPower = 0.0;

    for (auto id : conflict.attackers) {
        attackerPower += world_.getCountry(id).military.calculatePowerIndex();
    }
    for (auto id : conflict.defenders) {
        defenderPower += world_.getCountry(id).military.calculatePowerIndex();
    }

    double totalPower = attackerPower + defenderPower;
    if (totalPower <= 0) return;

    // Defensor tem vantagem (1.2x)
    defenderPower *= 1.2;

    // Baixas baseadas no poder relativo
    double attackerRatio = attackerPower / totalPower;
    double defenderRatio = defenderPower / totalPower;

    double baseCasualties = 100.0 * dt; // Base de baixas por dia

    for (auto id : conflict.attackers) {
        double cas = baseCasualties * defenderRatio * RandomEngine::instance().randDouble(0.5, 1.5);
        conflict.casualties[id] += cas;
        world_.getCountry(id).military.activePersonnel -= cas;
    }
    for (auto id : conflict.defenders) {
        double cas = baseCasualties * attackerRatio * RandomEngine::instance().randDouble(0.5, 1.5) * 0.8;
        conflict.casualties[id] += cas;
        world_.getCountry(id).military.activePersonnel -= cas;
    }

    // Atualizar progresso da frente
    conflict.frontProgress += (attackerPower - defenderPower) * 0.001 * dt;
    conflict.frontProgress = std::clamp(conflict.frontProgress, -1.0, 1.0);
}

void MilitarySystem::calculateBattleOutcome(MilitaryConflict& conflict) {
    // Se uma side está dominando completamente
    if (std::abs(conflict.frontProgress) > 0.9) {
        // Verificar se o lado perdedor quer capitular
        auto& losers = conflict.frontProgress > 0 ? conflict.defenders : conflict.attackers;
        for (auto id : losers) {
            auto& country = world_.getCountry(id);
            if (country.military.activePersonnel < 10000 ||
                warWeariness_[id] > 0.9) {
                // País está prestes a capitular
                country.internalStability -= 0.1;
            }
        }
    }
}

void MilitarySystem::checkEscalation(MilitaryConflict& conflict) {
    // Verificar se há arsenal nuclear envolvido
    for (auto id : conflict.attackers) {
        if (world_.getCountry(id).military.nuclearWarheads > 0) {
            if (conflict.frontProgress < -0.7) { // Se perdendo feio
                conflict.nuclearThreat = true;
                conflict.escalationLevel = std::max(conflict.escalationLevel, 0.8);
            }
        }
    }
    for (auto id : conflict.defenders) {
        if (world_.getCountry(id).military.nuclearWarheads > 0) {
            if (conflict.frontProgress > 0.7) {
                conflict.nuclearThreat = true;
                conflict.escalationLevel = std::max(conflict.escalationLevel, 0.8);
            }
        }
    }
}

void MilitarySystem::updateUnits(double dt) {
    for (auto& [id, unit] : units_) {
        // Recuperação de moral e força quando não em combate
        if (!unit.inCombat) {
            unit.morale += 0.005 * dt;
            unit.strength += 0.002 * dt;
            unit.supply += 0.01 * dt;
        } else {
            unit.morale -= 0.01 * dt;
            unit.supply -= 0.02 * dt;
        }

        unit.morale = std::clamp(unit.morale, 0.0, 1.0);
        unit.strength = std::clamp(unit.strength, 0.0, 1.0);
        unit.supply = std::clamp(unit.supply, 0.0, 1.0);
    }
}

void MilitarySystem::updateOperations(double dt) {
    for (auto& [id, op] : operations_) {
        if (!op.active) continue;

        op.progress += dt / 30.0; // Simplificado
        if (op.progress >= 1.0) {
            op.active = false;
        }
    }
}

// ===== Ações do Jogador =====

UnitID MilitarySystem::createUnit(CountryID country, MilitaryBranch branch,
                                   const std::string& name, int personnel) {
    MilitaryUnit unit;
    unit.id = nextUnitId_++;
    unit.country = country;
    unit.branch = branch;
    unit.name = name;
    unit.personnel = personnel;
    unit.strength = 1.0;
    unit.morale = 0.7;
    unit.experience = 0.1;
    units_[unit.id] = unit;

    auto& mil = world_.getCountry(country).military;
    mil.activePersonnel += personnel;

    return unit.id;
}

void MilitarySystem::disbandUnit(UnitID unitId) {
    auto it = units_.find(unitId);
    if (it == units_.end()) return;

    auto& mil = world_.getCountry(it->second.country).military;
    mil.activePersonnel -= it->second.personnel;
    units_.erase(it);
}

void MilitarySystem::deployUnit(UnitID unitId, GeoCoord destination) {
    auto it = units_.find(unitId);
    if (it != units_.end()) {
        it->second.deployed = true;
        it->second.position = destination;
    }
}

void MilitarySystem::recallUnit(UnitID unitId) {
    auto it = units_.find(unitId);
    if (it != units_.end()) {
        it->second.deployed = false;
        it->second.inCombat = false;
    }
}

OperationID MilitarySystem::launchOperation(CountryID country, MilitaryOperation::Type type,
                                             std::vector<UnitID> unitIds, GeoCoord target,
                                             const std::string& name) {
    MilitaryOperation op;
    op.id = nextOpId_++;
    op.country = country;
    op.type = type;
    op.units = std::move(unitIds);
    op.target = target;
    op.name = name;
    op.active = true;
    operations_[op.id] = op;
    return op.id;
}

void MilitarySystem::cancelOperation(OperationID opId) {
    auto it = operations_.find(opId);
    if (it != operations_.end()) {
        it->second.active = false;
    }
}

void MilitarySystem::setMilitaryBudget(CountryID country, double gdpShare) {
    auto& mil = world_.getCountry(country).military;
    mil.gdpShare = std::clamp(gdpShare, 0.0, 0.25); // Max 25% do PIB
    mil.annualBudget = Money(world_.getCountry(country).gdp.billions * mil.gdpShare);
}

void MilitarySystem::conscript(CountryID country, int number) {
    auto& c = world_.getCountry(country);
    c.military.activePersonnel += number;
    // Impacto negativo na opinião pública
    c.governmentApproval -= number * 0.00001;
}

void MilitarySystem::demobilize(CountryID country, int number) {
    auto& c = world_.getCountry(country);
    double actual = std::min(static_cast<double>(number), c.military.activePersonnel * 0.5);
    c.military.activePersonnel -= actual;
}

void MilitarySystem::purchaseEquipment(CountryID country, MilitaryBranch branch,
                                        int quantity, Money cost) {
    auto& mil = world_.getCountry(country).military;
    auto& c = world_.getCountry(country);
    // Não compra se não tiver reservas suficientes
    if (c.foreignReserves.billions < cost.billions) return;
    c.foreignReserves -= cost;
    if (c.foreignReserves.billions < 0.0) c.foreignReserves = Money(0.0);

    switch (branch) {
        case MilitaryBranch::ARMY:
            mil.tanks += quantity;
            break;
        case MilitaryBranch::AIR_FORCE:
            mil.fighters += quantity;
            break;
        case MilitaryBranch::NAVY:
            mil.warships += quantity;
            break;
        default:
            break;
    }
}

void MilitarySystem::buildNuclearWeapon(CountryID country) {
    auto& c = world_.getCountry(country);
    c.military.nuclearWarheads++;
    // Enorme impacto diplomático
    c.softPower -= 0.1;
    for (auto id : world_.getAllCountryIds()) {
        if (id == country) continue;
        auto& rel = world_.getRelation(country, id);
        rel.relations -= 0.05;
    }
}

uint32_t MilitarySystem::startConflict(const std::string& name, std::vector<CountryID> attackers,
                                        std::vector<CountryID> defenders, MilitaryConflict::Type type) {
    MilitaryConflict conflict;
    conflict.id = nextConflictId_++;
    conflict.name = name;
    conflict.attackers = std::move(attackers);
    conflict.defenders = std::move(defenders);
    conflict.type = type;
    conflict.active = true;
    conflicts_.push_back(conflict);
    return conflict.id;
}

void MilitarySystem::endConflict(uint32_t conflictId) {
    for (auto& c : conflicts_) {
        if (c.id == conflictId) {
            c.active = false;
            break;
        }
    }
}

// ===== Consultas =====

MilitaryForces MilitarySystem::getForces(CountryID country) const {
    return world_.getCountry(country).military;
}

std::vector<MilitaryUnit> MilitarySystem::getUnits(CountryID country) const {
    std::vector<MilitaryUnit> result;
    for (const auto& [id, unit] : units_) {
        if (unit.country == country) result.push_back(unit);
    }
    return result;
}

std::vector<MilitaryConflict> MilitarySystem::getActiveConflicts() const {
    std::vector<MilitaryConflict> result;
    for (const auto& c : conflicts_) {
        if (c.active) result.push_back(c);
    }
    return result;
}

double MilitarySystem::getMilitaryPower(CountryID country) const {
    return world_.getCountry(country).military.calculatePowerIndex();
}

double MilitarySystem::getWarWeariness(CountryID country) const {
    auto it = warWeariness_.find(country);
    return it != warWeariness_.end() ? it->second : 0.0;
}

bool MilitarySystem::isAtWar(CountryID country) const {
    for (const auto& c : conflicts_) {
        if (!c.active) continue;
        for (auto id : c.attackers) if (id == country) return true;
        for (auto id : c.defenders) if (id == country) return true;
    }
    return false;
}

double MilitarySystem::getConflictCost(CountryID country) const {
    double total = 0.0;
    for (const auto& c : conflicts_) {
        if (!c.active) continue;
        for (auto id : c.attackers) if (id == country) total += c.totalCost.billions;
        for (auto id : c.defenders) if (id == country) total += c.totalCost.billions;
    }
    return total;
}

double MilitarySystem::getNuclearDeterrence(CountryID country) const {
    auto& mil = world_.getCountry(country).military;
    if (mil.nuclearWarheads == 0) return 0.0;
    return std::min(1.0, std::log10(std::max(1, mil.nuclearWarheads)) / 3.0);
}

std::vector<MilitaryConflict> MilitarySystem::getConflictsOf(CountryID country) const {
    std::vector<MilitaryConflict> result;
    for (const auto& c : conflicts_) {
        for (auto id : c.attackers) if (id == country) { result.push_back(c); break; }
        for (auto id : c.defenders) if (id == country) { result.push_back(c); break; }
    }
    return result;
}

double MilitarySystem::calculateUnitPower(const MilitaryUnit& unit) const {
    return unit.personnel * unit.strength * unit.morale * unit.experience * unit.supply;
}

void MilitarySystem::joinConflict(uint32_t conflictId, CountryID country, bool asAttacker) {
    for (auto& c : conflicts_) {
        if (c.id == conflictId) {
            if (asAttacker) c.attackers.push_back(country);
            else c.defenders.push_back(country);
            break;
        }
    }
}

} // namespace GPS
