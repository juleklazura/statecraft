/**
 * GPS - Geopolitical Simulator
 * PoliticsSystem.h - Sistema político e de governo
 * 
 * Gerencia poder executivo, legislativo, judiciário,
 * oposição, mídia, estabilidade institucional e crises políticas.
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct GovernmentAction {
    enum class Type {
        PROPOSE_LAW,
        REVOKE_LAW,
        CABINET_RESHUFFLE,
        EMERGENCY_DECREE,
        DISSOLVE_PARLIAMENT,
        CALL_REFERENDUM,
        APPOINT_OFFICIAL,
        FIRE_OFFICIAL,
        DECLARE_STATE_OF_EMERGENCY,
        END_STATE_OF_EMERGENCY,
        AMNESTY,
        CENSORSHIP,
        MEDIA_CONTROL,
        MILITARY_MOBILIZATION,
        DEMOBILIZATION
    };

    Type type;
    CountryID country;
    std::string description;
    SimDate date;
    bool successful = false;
    double popularityImpact = 0.0;
    double stabilityImpact = 0.0;
};

struct PoliticalCrisis {
    enum class Type {
        IMPEACHMENT,
        VOTE_OF_NO_CONFIDENCE,
        COUP_ATTEMPT,
        MASS_PROTESTS,
        GENERAL_STRIKE,
        CONSTITUTIONAL_CRISIS,
        ASSASSINATION_ATTEMPT,
        SCANDAL,
        CIVIL_UNREST,
        REVOLUTION
    };

    Type type;
    CountryID country;
    SimDate startDate;
    double severity = 0.5;      // 0-1
    double momentum = 0.0;      // Positivo = escalando
    bool resolved = false;
    std::string description;
    std::vector<PopGroupID> involvedGroups;
};

struct ParliamentState {
    int totalSeats = 0;
    std::map<PartyID, int> seatDistribution;
    double governmentMajority = 0.0;  // % das cadeiras
    bool hasSupermajority = false;
    double oppositionStrength = 0.5;
    int pendingBills = 0;
    int approvedBills = 0;
    int rejectedBills = 0;
};

struct MediaState {
    double governmentFavorability = 0.5;
    double pressLiberty = 0.5;
    double mediaTrust = 0.5;
    double propagandaLevel = 0.0;
    double investigativeJournalism = 0.5;
    std::vector<std::string> recentHeadlines;
};

class PoliticsSystem : public ISystem {
public:
    explicit PoliticsSystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "PoliticsSystem"; }
    int getPriority() const override { return 20; }

    // Ações do jogador
    bool executeGovernmentAction(const GovernmentAction& action);
    bool proposeLaw(CountryID country, LawID lawId);
    bool revokeLaw(CountryID country, LawID lawId);
    void reshuffleCabinet(CountryID country);
    void declareEmergency(CountryID country);
    void endEmergency(CountryID country);
    void callReferendum(CountryID country, const std::string& question);
    void adjustMediaControl(CountryID country, double level);

    // Consultas
    ParliamentState getParliamentState(CountryID country) const;
    MediaState getMediaState(CountryID country) const;
    double getGovernmentApproval(CountryID country) const;
    double getInstitutionalStability(CountryID country) const;
    std::vector<PoliticalCrisis> getActiveCrises(CountryID country) const;
    double getCoupRisk(CountryID country) const;
    double getImpeachmentRisk(CountryID country) const;
    double getRevolutionRisk(CountryID country) const;
    bool isInEmergency(CountryID country) const;

private:
    void updatePoliticalStability(Country& country, double dt);
    void updateParliament(Country& country, double dt);
    void updateMedia(Country& country, double dt);
    void updateOpposition(Country& country, double dt);
    void checkForCrises(Country& country, const SimDate& date);
    void resolveCrises(Country& country, double dt);
    void calculateGovernmentApproval(Country& country);

    WorldState& world_;
    const SimulationConfig& config_;

    std::unordered_map<CountryID, ParliamentState> parliaments_;
    std::unordered_map<CountryID, MediaState> media_;
    std::unordered_map<CountryID, std::vector<PoliticalCrisis>> crises_;
    std::unordered_map<CountryID, std::vector<GovernmentAction>> actionHistory_;
    std::unordered_map<CountryID, bool> emergencyStates_;
};

} // namespace GPS
