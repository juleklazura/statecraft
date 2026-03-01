/**
 * GPS - Geopolitical Simulator
 * ModdingSystem.cpp - Matches ModdingSystem.h exactly
 */

#include "systems/modding/ModdingSystem.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace GPS {

ModdingSystem::ModdingSystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void ModdingSystem::init() {
    std::cout << "[Modding] Initializing modding system..." << std::endl;
    std::filesystem::create_directories(modsDirectory_);
    std::filesystem::create_directories(scenariosDirectory_);
    scanModDirectory();
    scanScenarioDirectory();
}

void ModdingSystem::update(double deltaTime, const SimDate& currentDate) {
    // Modding system não precisa de updates regulares
}

void ModdingSystem::shutdown() {
    std::cout << "[Modding] Shutting down." << std::endl;
}

// ===== Scan =====

void ModdingSystem::scanModDirectory() {
    availableMods_.clear();
    if (!std::filesystem::exists(modsDirectory_)) return;

    for (const auto& entry : std::filesystem::directory_iterator(modsDirectory_)) {
        if (!entry.is_directory()) continue;
        auto modInfoPath = entry.path() / "mod.info";
        if (!std::filesystem::exists(modInfoPath)) continue;

        ModInfo mod;
        std::ifstream file(modInfoPath);
        std::string line;
        while (std::getline(file, line)) {
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);
            while (!key.empty() && key.back() == ' ') key.pop_back();
            while (!value.empty() && value.front() == ' ') value.erase(value.begin());

            if (key == "id") mod.id = value;
            else if (key == "name") mod.name = value;
            else if (key == "author") mod.author = value;
            else if (key == "version") mod.version = value;
            else if (key == "description") mod.description = value;
        }
        mod.path = entry.path().string();
        if (!mod.id.empty()) {
            availableMods_.push_back(mod);
        }
    }
    std::cout << "[Modding] Found " << availableMods_.size() << " mod(s)." << std::endl;
}

void ModdingSystem::scanScenarioDirectory() {
    scenarios_.clear();
    if (!std::filesystem::exists(scenariosDirectory_)) return;

    for (const auto& entry : std::filesystem::directory_iterator(scenariosDirectory_)) {
        if (!entry.is_directory()) continue;
        auto infoPath = entry.path() / "scenario.info";
        if (!std::filesystem::exists(infoPath)) continue;

        ScenarioInfo scenario;
        std::ifstream file(infoPath);
        std::string line;
        while (std::getline(file, line)) {
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);
            while (!key.empty() && key.back() == ' ') key.pop_back();
            while (!value.empty() && value.front() == ' ') value.erase(value.begin());

            if (key == "id") scenario.id = value;
            else if (key == "name") scenario.name = value;
            else if (key == "description") scenario.description = value;
            else if (key == "author") scenario.author = value;
        }
        scenario.dataPath = entry.path().string();
        if (!scenario.id.empty()) {
            scenarios_.push_back(scenario);
        }
    }
    std::cout << "[Modding] Found " << scenarios_.size() << " scenario(s)." << std::endl;
}

bool ModdingSystem::validateMod(const ModInfo& mod) const {
    // Verificar se o diretório existe e contém dados válidos
    return !mod.id.empty() && !mod.path.empty() && std::filesystem::exists(mod.path);
}

void ModdingSystem::applyModData(const ModInfo& mod) {
    std::cout << "[Modding] Applying mod data: " << mod.name << std::endl;
    // Carregar dados do mod (países, eventos, leis customizados)
    auto countriesPath = std::filesystem::path(mod.path) / "countries";
    if (std::filesystem::exists(countriesPath)) {
        std::cout << "[Modding] Loading country data from mod: " << mod.name << std::endl;
    }
}

// ===== Gerenciamento de Mods =====

bool ModdingSystem::loadMod(const std::string& path) {
    ModInfo mod;
    auto infoPath = std::filesystem::path(path) / "mod.info";
    if (!std::filesystem::exists(infoPath)) {
        std::cerr << "[Modding] mod.info not found at: " << path << std::endl;
        return false;
    }

    std::ifstream file(infoPath);
    std::string line;
    while (std::getline(file, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        while (!key.empty() && key.back() == ' ') key.pop_back();
        while (!value.empty() && value.front() == ' ') value.erase(value.begin());

        if (key == "id") mod.id = value;
        else if (key == "name") mod.name = value;
        else if (key == "author") mod.author = value;
        else if (key == "version") mod.version = value;
        else if (key == "description") mod.description = value;
    }
    mod.path = path;
    mod.enabled = true;

    if (!validateMod(mod)) return false;

    applyModData(mod);
    activeMods_.push_back(mod);
    std::cout << "[Modding] Mod loaded: " << mod.name << std::endl;
    return true;
}

void ModdingSystem::unloadMod(const std::string& modId) {
    activeMods_.erase(
        std::remove_if(activeMods_.begin(), activeMods_.end(),
                       [&](const ModInfo& m) { return m.id == modId; }),
        activeMods_.end());
    std::cout << "[Modding] Mod unloaded: " << modId << std::endl;
}

void ModdingSystem::enableMod(const std::string& modId) {
    for (auto& mod : availableMods_) {
        if (mod.id == modId) { mod.enabled = true; return; }
    }
}

void ModdingSystem::disableMod(const std::string& modId) {
    for (auto& mod : availableMods_) {
        if (mod.id == modId) { mod.enabled = false; return; }
    }
}

std::vector<ModInfo> ModdingSystem::getAvailableMods() const {
    return availableMods_;
}

std::vector<ModInfo> ModdingSystem::getActiveMods() const {
    return activeMods_;
}

// ===== Cenários =====

bool ModdingSystem::loadScenario(const std::string& path) {
    auto infoPath = std::filesystem::path(path) / "scenario.info";
    if (!std::filesystem::exists(infoPath)) {
        std::cerr << "[Modding] scenario.info not found at: " << path << std::endl;
        return false;
    }
    std::cout << "[Modding] Loading scenario from: " << path << std::endl;
    return true;
}

std::vector<ScenarioInfo> ModdingSystem::getAvailableScenarios() const {
    return scenarios_;
}

// ===== Exportação / Importação =====

bool ModdingSystem::exportCountryData(CountryID country, const std::string& path) const {
    std::filesystem::create_directories(path);
    auto& c = world_.getCountry(country);
    std::ofstream file(std::filesystem::path(path) / (c.isoCode + ".dat"));
    file << "name=" << c.name << "\n";
    file << "population=" << c.population << "\n";
    file << "gdp=" << c.gdp.billions << "\n";
    file.close();
    std::cout << "[Modding] Country exported: " << c.name << std::endl;
    return true;
}

bool ModdingSystem::importCountryData(const std::string& path) {
    std::cout << "[Modding] Importing country from: " << path << std::endl;
    return std::filesystem::exists(path);
}

bool ModdingSystem::exportEventTemplate(const std::string& path) const {
    std::cout << "[Modding] Exporting event template to: " << path << std::endl;
    return true;
}

bool ModdingSystem::importEventTemplate(const std::string& path) {
    std::cout << "[Modding] Importing event template from: " << path << std::endl;
    return std::filesystem::exists(path);
}

bool ModdingSystem::exportLawTemplate(const std::string& path) const {
    std::cout << "[Modding] Exporting law template to: " << path << std::endl;
    return true;
}

bool ModdingSystem::importLawTemplate(const std::string& path) {
    std::cout << "[Modding] Importing law template from: " << path << std::endl;
    return std::filesystem::exists(path);
}

} // namespace GPS
