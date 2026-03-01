/**
 * GPS - Geopolitical Simulator
 * EnvironmentSystem.cpp
 */

#include "systems/environment/EnvironmentSystem.h"
#include <iostream>
#include <cmath>

namespace GPS {

EnvironmentSystem::EnvironmentSystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void EnvironmentSystem::init() {
    std::cout << "[Environment] Initializing climate simulation..." << std::endl;
    climate_.globalTemperatureAnomaly = 1.1;
    climate_.globalCO2ppm = 420.0;

    for (auto id : world_.getAllCountryIds()) {
        EnvironmentalPolicy policy;
        auto& c = world_.getCountry(id);
        policy.carbonTax = c.carbonTax;
        policy.renewableIncentive = c.renewableShare * 0.5;
        policies_[id] = policy;
    }
}

void EnvironmentSystem::update(double deltaTime, const SimDate& currentDate) {
    if (!config_.enableClimateChange) return;

    if (currentDate.hour == 0) { // Diariamente
        for (auto id : world_.getAllCountryIds()) {
            updateCountryEnvironment(world_.getCountry(id), deltaTime);
        }
        checkForDisasters(currentDate);
        processDisasters(deltaTime);
    }

    // Clima global atualizado mensalmente
    if (currentDate.day == 1 && currentDate.hour == 0) {
        updateClimate(deltaTime);
        calculateEmissions();
        applyClimateEffects();
    }
}

void EnvironmentSystem::shutdown() {
    std::cout << "[Environment] Shutting down." << std::endl;
}

void EnvironmentSystem::updateClimate(double dt) {
    double totalCO2 = world_.getGlobalCO2();

    // CO2 atmosférico aumenta com emissões
    climate_.globalCO2ppm += totalCO2 * 0.0001; // Simplificado

    // Temperatura aumenta com CO2
    double co2Effect = (climate_.globalCO2ppm - 280.0) / 280.0; // Pré-industrial = 280ppm
    climate_.globalTemperatureAnomaly = co2Effect * 3.0; // Sensibilidade climática ~3°C por duplicação

    // Nível do mar
    climate_.seaLevelRise = climate_.globalTemperatureAnomaly * 0.0074; // ~7.4mm/°C/ano

    // Gelo ártico
    climate_.arcticIceExtent = std::clamp(1.0 - climate_.globalTemperatureAnomaly * 0.2, 0.0, 1.0);

    // Acidificação oceânica
    climate_.oceanAcidification = std::clamp((climate_.globalCO2ppm - 280.0) / 500.0, 0.0, 1.0);

    world_.setGlobalTemperatureAnomaly(climate_.globalTemperatureAnomaly);
}

void EnvironmentSystem::updateCountryEnvironment(Country& country, double dt) {
    auto& policy = policies_[country.id];

    // Emissões baseadas em economia e energia
    double baseEmissions = country.gdp.billions * 0.5; // Megatons
    double policyReduction = policy.carbonTax * 0.3 +
                             policy.emissionCap * 0.2 +
                             country.renewableShare * 0.3;
    country.co2EmissionsMT = baseEmissions * (1.0 - policyReduction * 0.5);

    // Qualidade do ar
    double pollution = country.co2EmissionsMT / (country.areaSqKm * 0.001);
    country.airQualityIndex = std::clamp(100.0 - pollution * 10.0, 0.0, 100.0);

    // Transição energética
    if (policy.renewableIncentive > 0) {
        country.renewableShare += policy.renewableIncentive * 0.0001 * dt;
        country.renewableShare = std::clamp(country.renewableShare, 0.0, 0.95);
    }

    // Desmatamento
    if (policy.deforestationPenalty > 0) {
        country.deforestationRate *= (1.0 - policy.deforestationPenalty * 0.01 * dt);
    }
}

void EnvironmentSystem::checkForDisasters(const SimDate& date) {
    for (auto id : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(id);

        // Probabilidade de desastre baseada em localização e clima
        double disasterProb = country.disasterRisk * 0.001;
        disasterProb *= (1.0 + climate_.globalTemperatureAnomaly * 0.2); // Mudanças climáticas aumentam risco

        if (RandomEngine::instance().chance(disasterProb)) {
            NaturalDisaster disaster;
            disaster.id = nextDisasterId_++;
            disaster.country = id;
            disaster.date = date;

            // Tipo aleatório
            int type = RandomEngine::instance().randInt(0, 9);
            disaster.type = static_cast<NaturalDisaster::Type>(type);

            // Severidade
            disaster.severity = RandomEngine::instance().randDouble(0.2, 0.9);

            switch (disaster.type) {
                case NaturalDisaster::Type::EARTHQUAKE:
                    disaster.name = "Terremoto";
                    disaster.durationDays = 1;
                    disaster.economicDamage = disaster.severity * country.gdp.billions * 0.05;
                    disaster.casualties = disaster.severity * 1000;
                    break;
                case NaturalDisaster::Type::HURRICANE:
                    disaster.name = "Furacão";
                    disaster.durationDays = 7;
                    disaster.economicDamage = disaster.severity * country.gdp.billions * 0.03;
                    disaster.casualties = disaster.severity * 200;
                    break;
                case NaturalDisaster::Type::FLOOD:
                    disaster.name = "Inundação";
                    disaster.durationDays = 14;
                    disaster.economicDamage = disaster.severity * country.gdp.billions * 0.02;
                    disaster.casualties = disaster.severity * 100;
                    disaster.displacedPopulation = disaster.severity * country.population * 0.01;
                    break;
                case NaturalDisaster::Type::DROUGHT:
                    disaster.name = "Seca";
                    disaster.durationDays = 90;
                    disaster.economicDamage = disaster.severity * country.gdp.billions * 0.04;
                    break;
                case NaturalDisaster::Type::WILDFIRE:
                    disaster.name = "Incêndio Florestal";
                    disaster.durationDays = 30;
                    disaster.economicDamage = disaster.severity * country.gdp.billions * 0.01;
                    break;
                default:
                    disaster.name = "Desastre Natural";
                    disaster.durationDays = 7;
                    disaster.economicDamage = disaster.severity * country.gdp.billions * 0.02;
                    break;
            }

            activeDisasters_.push_back(disaster);
        }
    }
}

void EnvironmentSystem::processDisasters(double dt) {
    for (auto& disaster : activeDisasters_) {
        if (!disaster.active) continue;

        disaster.durationDays -= static_cast<int>(dt);
        if (disaster.durationDays <= 0) {
            disaster.active = false;
            disasterHistory_.push_back(disaster);
            continue;
        }

        // Aplicar danos contínuos
        auto& country = world_.getCountry(disaster.country);
        country.gdp.billions -= disaster.economicDamage * dt / 365.0;
        country.internalStability -= disaster.severity * 0.005 * dt;
    }

    // Limpar desastres inativos
    activeDisasters_.erase(
        std::remove_if(activeDisasters_.begin(), activeDisasters_.end(),
                       [](const NaturalDisaster& d) { return !d.active; }),
        activeDisasters_.end()
    );
}

void EnvironmentSystem::calculateEmissions() {
    double total = 0;
    for (auto id : world_.getAllCountryIds()) {
        total += world_.getCountry(id).co2EmissionsMT;
    }
    climate_.globalCO2ppm += total * 0.00001; // Aproximação
}

void EnvironmentSystem::applyClimateEffects() {
    // Efeitos globais das mudanças climáticas
    if (climate_.globalTemperatureAnomaly > 2.0) {
        // Aumentar risco de desastres em todos os países
        for (auto id : world_.getAllCountryIds()) {
            auto& c = world_.getCountry(id);
            c.disasterRisk += 0.001;
        }
    }
}

// ===== Ações do Jogador =====

void EnvironmentSystem::setEnvironmentalPolicy(CountryID country, const EnvironmentalPolicy& policy) {
    policies_[country] = policy;
    auto& c = world_.getCountry(country);
    c.carbonTax = policy.carbonTax;
}

void EnvironmentSystem::investInRenewables(CountryID country, Money amount) {
    auto& c = world_.getCountry(country);
    double actual = std::min(amount.billions, c.foreignReserves.billions);
    if (actual <= 0.0) return;
    c.foreignReserves -= Money(actual);
    if (c.foreignReserves.billions < 0.0) c.foreignReserves = Money(0.0);
    c.renewableShare += actual * 0.01;
    c.renewableShare = std::clamp(c.renewableShare, 0.0, 0.95);
}

void EnvironmentSystem::joinClimateAgreement(CountryID country) {
    policies_[country].parisAgreementMember = true;
    world_.getCountry(country).softPower += 0.02;
}

void EnvironmentSystem::leaveClimateAgreement(CountryID country) {
    policies_[country].parisAgreementMember = false;
    world_.getCountry(country).softPower -= 0.05;
}

void EnvironmentSystem::createNationalPark(CountryID country, RegionID region) {
    auto& c = world_.getCountry(country);
    c.deforestationRate *= 0.9;
    c.softPower += 0.01;
}

void EnvironmentSystem::banDeforestation(CountryID country) {
    world_.getCountry(country).deforestationRate = 0.0;
    policies_[country].deforestationPenalty = 1.0;
}

// ===== Consultas =====

ClimateData EnvironmentSystem::getClimateData() const { return climate_; }

EnvironmentalPolicy EnvironmentSystem::getPolicy(CountryID country) const {
    auto it = policies_.find(country);
    if (it != policies_.end()) return it->second;
    return {};
}

std::vector<NaturalDisaster> EnvironmentSystem::getActiveDisasters() const {
    return activeDisasters_;
}

std::vector<NaturalDisaster> EnvironmentSystem::getDisastersIn(CountryID country) const {
    std::vector<NaturalDisaster> result;
    for (const auto& d : activeDisasters_) {
        if (d.country == country) result.push_back(d);
    }
    return result;
}

double EnvironmentSystem::getCountryEmissions(CountryID country) const {
    return world_.getCountry(country).co2EmissionsMT;
}

double EnvironmentSystem::getPollutionLevel(CountryID country) const {
    return 100.0 - world_.getCountry(country).airQualityIndex;
}

double EnvironmentSystem::getDisasterRisk(CountryID country) const {
    return world_.getCountry(country).disasterRisk;
}

double EnvironmentSystem::getEnvironmentalScore(CountryID country) const {
    auto& c = world_.getCountry(country);
    return (c.airQualityIndex / 100.0 * 0.3 +
            c.renewableShare * 0.3 +
            (1.0 - c.deforestationRate) * 0.2 +
            c.waterQualityIndex * 0.2);
}

} // namespace GPS
