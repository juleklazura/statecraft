/**
 * GPS - Geopolitical Simulator
 * PopulationSystem.h - Sistema de população e sociedade
 * 
 * Gerencia grupos sociais, opinião pública, demografia,
 * satisfação, radicalização e movimentos sociais.
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct DemographicData {
    double totalPopulation = 0.0;
    double birthRate = 0.02;
    double deathRate = 0.008;
    double immigrationRate = 0.001;
    double emigrationRate = 0.001;
    double fertilePopulationShare = 0.25;
    double workingAgeShare = 0.65;
    double dependencyRatio = 0.5;

    // Pirâmide etária simplificada
    double age0to14 = 0.25;
    double age15to24 = 0.15;
    double age25to54 = 0.40;
    double age55to64 = 0.10;
    double age65plus = 0.10;
};

struct PublicOpinion {
    double governmentApproval = 0.5;
    double economicOptimism = 0.5;
    double securityPerception = 0.5;
    double nationalPride = 0.5;
    double trustInInstitutions = 0.5;
    double trustInMedia = 0.5;
    double socialCohesion = 0.5;
    double environmentalConcern = 0.3;
    double immigrationSentiment = 0.5;  // 0=contra, 1=a favor
    double warSupport = 0.3;
    double overallHappiness = 0.5;

    // Trending
    double approvalTrend = 0.0;
    double satisfactionTrend = 0.0;
};

struct SocialMovement {
    std::string name;
    CountryID country;
    SocialGroupType mainGroup;
    double support = 0.0;        // % da população
    double momentum = 0.0;      // Velocidade de crescimento
    double radicalization = 0.0;
    bool isViolent = false;
    SimDate startDate;
    std::string cause;

    enum class Type {
        PROTEST,
        STRIKE,
        BOYCOTT,
        CIVIL_DISOBEDIENCE,
        ARMED_RESISTANCE,
        SEPARATISM,
        REVOLUTION
    } type;
};

class PopulationSystem : public ISystem {
public:
    explicit PopulationSystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "PopulationSystem"; }
    int getPriority() const override { return 15; }

    // Consultas
    DemographicData getDemographics(CountryID country) const;
    PublicOpinion getPublicOpinion(CountryID country) const;
    std::vector<SocialMovement> getActiveMovements(CountryID country) const;
    double getGroupSatisfaction(CountryID country, SocialGroupType group) const;
    double getGroupRadicalization(CountryID country, SocialGroupType group) const;
    double getRevoltRisk(CountryID country) const;
    double getUrbanRuralTension(CountryID country) const;
    double getEthnicTension(CountryID country) const;
    double getReligiousTension(CountryID country) const;

    // Impactos externos
    void applyPolicyImpact(CountryID country, SocialGroupType group,
                           double satisfactionDelta, double radicalizationDelta);
    void applyEconomicShock(CountryID country, double magnitude);
    void triggerSocialMovement(CountryID country, SocialMovement movement);

private:
    void updateDemographics(Country& country, double dt);
    void updatePublicOpinion(Country& country, double dt);
    void updateSocialGroups(Country& country, double dt);
    void updateSocialMovements(Country& country, double dt);
    void calculateGroupSatisfaction(Country& country, SocialGroup& group);
    void checkForUnrest(Country& country);
    void processImmigration(Country& country, double dt);

    WorldState& world_;
    const SimulationConfig& config_;

    std::unordered_map<CountryID, DemographicData> demographics_;
    std::unordered_map<CountryID, PublicOpinion> opinions_;
    std::unordered_map<CountryID, std::vector<SocialMovement>> movements_;
};

} // namespace GPS
