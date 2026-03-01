/**
 * GPS - Geopolitical Simulator
 * InfrastructureSystem.cpp
 */

#include "systems/infrastructure/InfrastructureSystem.h"
#include <iostream>

namespace GPS {

InfrastructureSystem::InfrastructureSystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void InfrastructureSystem::init() {
    std::cout << "[Infrastructure] Initializing infrastructure system..." << std::endl;
}

void InfrastructureSystem::update(double deltaTime, const SimDate& currentDate) {
    if (currentDate.hour == 0) {
        updateProjects(deltaTime);
        updateBuildings(deltaTime);
    }
}

void InfrastructureSystem::shutdown() {
    std::cout << "[Infrastructure] Shutting down." << std::endl;
}

void InfrastructureSystem::updateProjects(double dt) {
    for (auto& [id, project] : projects_) {
        if (project.completed || project.cancelled) continue;

        project.daysElapsed += static_cast<int>(dt);
        project.progress = static_cast<double>(project.daysElapsed) / project.buildTimeDays;
        project.spent += Money(project.cost.billions * dt / project.buildTimeDays);

        // Empregos durante construção
        auto& country = world_.getCountry(project.country);
        country.unemploymentRate -= project.jobsCreated * 0.000001 * dt;

        if (project.progress >= 1.0) {
            completeProject(project);
        }
    }
}

void InfrastructureSystem::completeProject(InfrastructureProject& project) {
    project.completed = true;
    project.progress = 1.0;

    auto& country = world_.getCountry(project.country);

    // Criar building
    Building building;
    building.id = project.id;
    building.country = project.country;
    building.region = project.region;
    building.type = project.type;
    building.name = project.name;
    building.condition = 1.0;
    building.efficiency = 1.0;
    building.maintenanceCost = project.maintenanceCostYearly;

    // Aplicar bônus
    country.gdp.billions += project.gdpBoost;

    if (project.region != INVALID_REGION && world_.getRegion(project.region).countryId == project.country) {
        auto& region = world_.getRegion(project.region);
        region.infrastructureLevel += project.regionDevBoost;
        region.infrastructureLevel = std::clamp(region.infrastructureLevel, 0.0, 1.0);
    }

    if (project.energyProduction > 0) {
        country.energyProductionTWh += project.energyProduction;
        building.output = project.energyProduction;
        building.capacity = project.energyProduction;
    }

    if (project.pollutionIncrease > 0) {
        country.co2EmissionsMT += project.pollutionIncrease;
    }

    buildings_[building.id] = building;
}

void InfrastructureSystem::updateBuildings(double dt) {
    for (auto& [id, building] : buildings_) {
        if (!building.operational) continue;

        // Degradação com o tempo
        building.condition -= 0.0001 * dt;
        if (building.condition < 0.3) {
            building.efficiency = building.condition;
        }
        if (building.condition <= 0) {
            building.operational = false;
        }
    }
}

// ===== Ações do Jogador =====

BuildingID InfrastructureSystem::startProject(CountryID country, RegionID region,
                                               InfrastructureType type, const std::string& name) {
    InfrastructureProject project;
    project.id = nextBuildingId_++;
    project.country = country;
    project.region = region;
    project.type = type;
    project.name = name;
    project.cost = estimateProjectCost(type, country);
    project.buildTimeDays = estimateBuildTime(type, country);

    switch (type) {
        case InfrastructureType::HIGHWAY:
            project.jobsCreated = 5000; project.gdpBoost = 0.5;
            project.regionDevBoost = 0.05; break;
        case InfrastructureType::RAILROAD:
            project.jobsCreated = 8000; project.gdpBoost = 0.8;
            project.regionDevBoost = 0.08; break;
        case InfrastructureType::PORT:
            project.jobsCreated = 3000; project.gdpBoost = 1.5;
            project.regionDevBoost = 0.1; break;
        case InfrastructureType::AIRPORT:
            project.jobsCreated = 4000; project.gdpBoost = 1.2;
            project.regionDevBoost = 0.07; break;
        case InfrastructureType::POWER_PLANT_NUCLEAR:
            project.jobsCreated = 2000; project.energyProduction = 10.0;
            project.maintenanceCostYearly = 0.5; break;
        case InfrastructureType::POWER_PLANT_SOLAR:
            project.jobsCreated = 1000; project.energyProduction = 3.0;
            project.maintenanceCostYearly = 0.1; break;
        case InfrastructureType::POWER_PLANT_WIND:
            project.jobsCreated = 800; project.energyProduction = 2.5;
            project.maintenanceCostYearly = 0.08; break;
        case InfrastructureType::POWER_PLANT_HYDRO:
            project.jobsCreated = 3000; project.energyProduction = 8.0;
            project.maintenanceCostYearly = 0.2; break;
        case InfrastructureType::POWER_PLANT_COAL:
            project.jobsCreated = 1500; project.energyProduction = 5.0;
            project.pollutionIncrease = 5.0;
            project.maintenanceCostYearly = 0.3; break;
        case InfrastructureType::HOSPITAL:
            project.jobsCreated = 2000; project.regionDevBoost = 0.05; break;
        case InfrastructureType::UNIVERSITY:
            project.jobsCreated = 1000; project.regionDevBoost = 0.06; break;
        case InfrastructureType::MILITARY_BASE:
            project.jobsCreated = 500; project.militaryValue = 0.1; break;
        case InfrastructureType::RESEARCH_CENTER:
            project.jobsCreated = 500; project.gdpBoost = 0.3; break;
        default:
            project.jobsCreated = 1000; project.regionDevBoost = 0.03; break;
    }

    projects_[project.id] = project;
    return project.id;
}

void InfrastructureSystem::cancelProject(BuildingID id) {
    auto it = projects_.find(id);
    if (it != projects_.end()) {
        it->second.cancelled = true;
        // Reembolso parcial
        auto& c = world_.getCountry(it->second.country);
        c.foreignReserves += Money(it->second.spent.billions * 0.3);
    }
}

void InfrastructureSystem::repairBuilding(BuildingID id, Money budget) {
    auto it = buildings_.find(id);
    if (it != buildings_.end()) {
        it->second.condition = std::min(1.0, it->second.condition + budget.billions * 0.1);
        it->second.operational = true;
        it->second.efficiency = it->second.condition;
    }
}

void InfrastructureSystem::upgradeBuilding(BuildingID id, Money budget) {
    auto it = buildings_.find(id);
    if (it != buildings_.end()) {
        it->second.capacity *= 1.2;
        it->second.output *= 1.1;
        it->second.condition = 1.0;
    }
}

void InfrastructureSystem::decommissionBuilding(BuildingID id) {
    buildings_.erase(id);
}

void InfrastructureSystem::accelerateProject(BuildingID id, Money additionalFunding) {
    auto it = projects_.find(id);
    if (it != projects_.end() && !it->second.completed) {
        double acceleration = additionalFunding.billions / it->second.cost.billions;
        it->second.buildTimeDays = static_cast<int>(it->second.buildTimeDays * (1.0 - acceleration * 0.3));
    }
}

Money InfrastructureSystem::estimateProjectCost(InfrastructureType type, CountryID country) const {
    double baseCost = 1.0;
    switch (type) {
        case InfrastructureType::HIGHWAY: baseCost = 2.0; break;
        case InfrastructureType::RAILROAD: baseCost = 5.0; break;
        case InfrastructureType::PORT: baseCost = 3.0; break;
        case InfrastructureType::AIRPORT: baseCost = 4.0; break;
        case InfrastructureType::POWER_PLANT_NUCLEAR: baseCost = 20.0; break;
        case InfrastructureType::POWER_PLANT_SOLAR: baseCost = 3.0; break;
        case InfrastructureType::POWER_PLANT_WIND: baseCost = 2.5; break;
        case InfrastructureType::POWER_PLANT_HYDRO: baseCost = 8.0; break;
        case InfrastructureType::HOSPITAL: baseCost = 1.5; break;
        case InfrastructureType::UNIVERSITY: baseCost = 1.0; break;
        case InfrastructureType::MILITARY_BASE: baseCost = 5.0; break;
        case InfrastructureType::SPACE_CENTER: baseCost = 30.0; break;
        default: baseCost = 2.0; break;
    }
    // Ajustar pelo custo de vida do país
    double costMultiplier = world_.getCountry(country).gdpPerCapita / 30000.0;
    return Money(baseCost * std::max(0.5, costMultiplier));
}

int InfrastructureSystem::estimateBuildTime(InfrastructureType type, CountryID country) const {
    int baseDays = 365;
    switch (type) {
        case InfrastructureType::HIGHWAY: baseDays = 730; break;
        case InfrastructureType::RAILROAD: baseDays = 1095; break;
        case InfrastructureType::PORT: baseDays = 1460; break;
        case InfrastructureType::AIRPORT: baseDays = 1825; break;
        case InfrastructureType::POWER_PLANT_NUCLEAR: baseDays = 3650; break;
        case InfrastructureType::POWER_PLANT_SOLAR: baseDays = 365; break;
        case InfrastructureType::HOSPITAL: baseDays = 730; break;
        case InfrastructureType::MILITARY_BASE: baseDays = 545; break;
        case InfrastructureType::SPACE_CENTER: baseDays = 2920; break;
        default: baseDays = 545; break;
    }
    return baseDays;
}

// ===== Consultas =====

std::vector<InfrastructureProject> InfrastructureSystem::getActiveProjects(CountryID country) const {
    std::vector<InfrastructureProject> result;
    for (const auto& [id, p] : projects_) {
        if (p.country == country && !p.completed && !p.cancelled) result.push_back(p);
    }
    return result;
}

std::vector<Building> InfrastructureSystem::getBuildings(CountryID country) const {
    std::vector<Building> result;
    for (const auto& [id, b] : buildings_) {
        if (b.country == country) result.push_back(b);
    }
    return result;
}

double InfrastructureSystem::getInfrastructureScore(CountryID country) const {
    double score = 0.0;
    int count = 0;
    for (const auto& [id, b] : buildings_) {
        if (b.country == country && b.operational) {
            score += b.condition * b.efficiency;
            count++;
        }
    }
    return count > 0 ? score / count : 0.5;
}

double InfrastructureSystem::getEnergyCapacity(CountryID country) const {
    double total = 0;
    for (const auto& [id, b] : buildings_) {
        if (b.country == country && b.operational) {
            total += b.capacity;
        }
    }
    return total;
}

Money InfrastructureSystem::getTotalMaintenanceCost(CountryID country) const {
    double total = 0;
    for (const auto& [id, b] : buildings_) {
        if (b.country == country && b.operational) {
            total += b.maintenanceCost;
        }
    }
    return Money(total);
}

InfrastructureProject InfrastructureSystem::getProjectInfo(BuildingID id) const {
    auto it = projects_.find(id);
    if (it != projects_.end()) return it->second;
    return {};
}

std::vector<Building> InfrastructureSystem::getBuildingsInRegion(RegionID region) const {
    std::vector<Building> result;
    for (const auto& [id, b] : buildings_) {
        if (b.region == region) result.push_back(b);
    }
    return result;
}

double InfrastructureSystem::getTransportScore(CountryID country) const {
    int transportBuildings = 0;
    for (const auto& [id, b] : buildings_) {
        if (b.country == country && b.operational) {
            if (b.type == InfrastructureType::HIGHWAY ||
                b.type == InfrastructureType::RAILROAD ||
                b.type == InfrastructureType::PORT ||
                b.type == InfrastructureType::AIRPORT) {
                transportBuildings++;
            }
        }
    }
    return std::min(1.0, transportBuildings / 50.0);
}

} // namespace GPS
