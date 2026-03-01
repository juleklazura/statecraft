/**
 * GPS - Geopolitical Simulator
 * ModdingSystem.h - Sistema de modding e criação de conteúdo
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct ModInfo {
    std::string id;
    std::string name;
    std::string author;
    std::string version;
    std::string description;
    std::string path;
    bool enabled = false;
    int loadOrder = 0;

    std::vector<std::string> dependencies;
    std::vector<std::string> conflicts;
};

struct ScenarioInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string author;
    SimDate startDate;
    std::string dataPath;

    // Condições de vitória opcionais
    std::vector<std::string> victoryConditions;
};

class ModdingSystem : public ISystem {
public:
    explicit ModdingSystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "ModdingSystem"; }
    int getPriority() const override { return 90; }

    // Gerenciamento de mods
    bool loadMod(const std::string& path);
    void unloadMod(const std::string& modId);
    void enableMod(const std::string& modId);
    void disableMod(const std::string& modId);
    std::vector<ModInfo> getAvailableMods() const;
    std::vector<ModInfo> getActiveMods() const;

    // Cenários
    bool loadScenario(const std::string& path);
    std::vector<ScenarioInfo> getAvailableScenarios() const;

    // Criação de conteúdo
    bool exportCountryData(CountryID country, const std::string& path) const;
    bool importCountryData(const std::string& path);
    bool exportEventTemplate(const std::string& path) const;
    bool importEventTemplate(const std::string& path);
    bool exportLawTemplate(const std::string& path) const;
    bool importLawTemplate(const std::string& path);

private:
    void scanModDirectory();
    void scanScenarioDirectory();
    bool validateMod(const ModInfo& mod) const;
    void applyModData(const ModInfo& mod);

    WorldState& world_;
    const SimulationConfig& config_;

    std::vector<ModInfo> availableMods_;
    std::vector<ModInfo> activeMods_;
    std::vector<ScenarioInfo> scenarios_;
    std::string modsDirectory_ = "data/mods/";
    std::string scenariosDirectory_ = "data/scenarios/";
};

} // namespace GPS
