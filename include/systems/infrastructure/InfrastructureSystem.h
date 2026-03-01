/**
 * GPS - Geopolitical Simulator
 * InfrastructureSystem.h - Infraestrutura e construção de obras
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct InfrastructureProject {
    BuildingID id;
    CountryID country;
    RegionID region;
    InfrastructureType type;
    std::string name;

    Money cost;
    Money spent;
    int buildTimeDays = 365;
    int daysElapsed = 0;
    double progress = 0.0;     // 0-1
    bool completed = false;
    bool cancelled = false;

    int jobsCreated = 0;
    double maintenanceCostYearly = 0.0;

    // Efeitos ao completar
    double gdpBoost = 0.0;
    double employmentBoost = 0.0;
    double regionDevBoost = 0.0;
    double energyProduction = 0.0;
    double pollutionIncrease = 0.0;
    double militaryValue = 0.0;
};

struct Building {
    BuildingID id;
    CountryID country;
    RegionID region;
    InfrastructureType type;
    std::string name;

    double condition = 1.0;    // 0-1, degrada com tempo
    double efficiency = 1.0;
    double maintenanceCost = 0.0;
    int yearBuilt = 2000;
    bool operational = true;

    // Produção (se aplicável)
    double output = 0.0;
    double capacity = 0.0;
};

class InfrastructureSystem : public ISystem {
public:
    explicit InfrastructureSystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "InfrastructureSystem"; }
    int getPriority() const override { return 55; }

    // Ações do jogador
    BuildingID startProject(CountryID country, RegionID region,
                            InfrastructureType type, const std::string& name);
    void cancelProject(BuildingID id);
    void accelerateProject(BuildingID id, Money additionalFunding);
    void decommissionBuilding(BuildingID id);
    void repairBuilding(BuildingID id, Money budget);
    void upgradeBuilding(BuildingID id, Money budget);

    // Consultas
    std::vector<InfrastructureProject> getActiveProjects(CountryID country) const;
    std::vector<Building> getBuildings(CountryID country) const;
    std::vector<Building> getBuildingsInRegion(RegionID region) const;
    double getInfrastructureScore(CountryID country) const;
    double getEnergyCapacity(CountryID country) const;
    double getTransportScore(CountryID country) const;
    Money getTotalMaintenanceCost(CountryID country) const;
    InfrastructureProject getProjectInfo(BuildingID id) const;

    // Cálculos de custos
    Money estimateProjectCost(InfrastructureType type, CountryID country) const;
    int estimateBuildTime(InfrastructureType type, CountryID country) const;

private:
    void updateProjects(double dt);
    void updateBuildings(double dt);
    void completeProject(InfrastructureProject& project);
    void degradeBuildings(double dt);
    void calculateInfrastructureEffects(Country& country);

    WorldState& world_;
    const SimulationConfig& config_;

    std::unordered_map<BuildingID, InfrastructureProject> projects_;
    std::unordered_map<BuildingID, Building> buildings_;
    BuildingID nextBuildingId_ = 1;
};

} // namespace GPS
