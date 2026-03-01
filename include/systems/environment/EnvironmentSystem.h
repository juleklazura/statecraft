/**
 * GPS - Geopolitical Simulator
 * EnvironmentSystem.h - Meio ambiente, clima e desastres naturais
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct ClimateData {
    double globalTemperatureAnomaly = 1.1;  // °C
    double seaLevelRise = 0.0;              // metros
    double arcticIceExtent = 1.0;           // 0-1 normalizado
    double globalCO2ppm = 420.0;
    double oceanAcidification = 0.0;        // 0-1
    double deforestationGlobalRate = 0.0;
};

struct NaturalDisaster {
    uint32_t id;
    CountryID country;
    RegionID region;
    SimDate date;
    double severity = 0.5;      // 0-1
    double economicDamage = 0.0; // Bilhões
    double casualties = 0.0;
    double displacedPopulation = 0.0;
    bool active = true;
    int durationDays = 1;

    enum class Type {
        EARTHQUAKE,
        TSUNAMI,
        HURRICANE,
        TORNADO,
        FLOOD,
        DROUGHT,
        WILDFIRE,
        VOLCANIC_ERUPTION,
        LANDSLIDE,
        HEATWAVE,
        COLDWAVE,
        PANDEMIC_NATURAL
    } type;

    std::string name;
};

struct EnvironmentalPolicy {
    double carbonTax = 0.0;
    double renewableIncentive = 0.0;
    double emissionCap = 1.0;        // 1 = sem limite
    double deforestationPenalty = 0.0;
    double pollutionRegulation = 0.5;
    double recyclingMandateStrength = 0.3;
    bool parisAgreementMember = true;
    double greenInvestment = 0.0;    // Bilhões/ano
};

class EnvironmentSystem : public ISystem {
public:
    explicit EnvironmentSystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "EnvironmentSystem"; }
    int getPriority() const override { return 60; }

    // Ações do jogador
    void setEnvironmentalPolicy(CountryID country, const EnvironmentalPolicy& policy);
    void investInRenewables(CountryID country, Money amount);
    void createNationalPark(CountryID country, RegionID region);
    void banDeforestation(CountryID country);
    void joinClimateAgreement(CountryID country);
    void leaveClimateAgreement(CountryID country);

    // Consultas
    ClimateData getClimateData() const;
    EnvironmentalPolicy getPolicy(CountryID country) const;
    std::vector<NaturalDisaster> getActiveDisasters() const;
    std::vector<NaturalDisaster> getDisastersIn(CountryID country) const;
    double getCountryEmissions(CountryID country) const;
    double getPollutionLevel(CountryID country) const;
    double getDisasterRisk(CountryID country) const;
    double getEnvironmentalScore(CountryID country) const;

private:
    void updateClimate(double dt);
    void updateCountryEnvironment(Country& country, double dt);
    void checkForDisasters(const SimDate& date);
    void processDisasters(double dt);
    void calculateEmissions();
    void applyClimateEffects();

    WorldState& world_;
    const SimulationConfig& config_;

    ClimateData climate_;
    std::unordered_map<CountryID, EnvironmentalPolicy> policies_;
    std::vector<NaturalDisaster> activeDisasters_;
    std::vector<NaturalDisaster> disasterHistory_;
    uint32_t nextDisasterId_ = 1;
};

} // namespace GPS
