/**
 * GPS - Geopolitical Simulator
 * DiplomacySystem.h - Sistema de diplomacia e relações internacionais
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct DiplomaticProposal {
    enum class Type {
        TRADE_AGREEMENT,
        MILITARY_ALLIANCE,
        NON_AGGRESSION_PACT,
        PEACE_TREATY,
        ARMS_DEAL,
        AID_PACKAGE,
        SANCTIONS,
        LIFT_SANCTIONS,
        EMBARGO,
        LIFT_EMBARGO,
        VISA_AGREEMENT,
        CULTURAL_EXCHANGE,
        TECHNOLOGY_SHARING,
        JOINT_MILITARY_EXERCISE,
        MEDIATION,
        CEASE_FIRE,
        DEMAND_TERRITORY,
        DEMAND_REPARATIONS,
        DENOUNCE,
        RECOGNIZE_GOVERNMENT,
        BREAK_RELATIONS,
        RESTORE_RELATIONS
    };

    Type type;
    CountryID proposer;
    CountryID target;
    std::string description;
    SimDate proposedDate;
    double acceptanceProbability = 0.5;
    bool accepted = false;
    bool pending = true;

    // Condições
    Money financialValue;
    int durationYears = 5;
};

struct UNResolution {
    uint32_t id;
    std::string title;
    std::string description;
    CountryID proposedBy;
    SimDate date;
    std::vector<CountryID> votedFor;
    std::vector<CountryID> votedAgainst;
    std::vector<CountryID> abstained;
    std::vector<CountryID> vetoedBy;
    bool passed = false;
};

class DiplomacySystem : public ISystem {
public:
    explicit DiplomacySystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "DiplomacySystem"; }
    int getPriority() const override { return 30; }

    // Ações do jogador
    void sendProposal(const DiplomaticProposal& proposal);
    void respondToProposal(uint32_t proposalId, bool accept);
    void declareSanctions(CountryID from, CountryID target);
    void liftSanctions(CountryID from, CountryID target);
    void declareWar(CountryID aggressor, CountryID target, const std::string& casusBelli);
    void proposePeace(CountryID from, CountryID to);
    void breakDiplomaticRelations(CountryID from, CountryID with);
    void restoreDiplomaticRelations(CountryID from, CountryID with);
    void joinOrganization(CountryID country, OrganizationID org);
    void leaveOrganization(CountryID country, OrganizationID org);
    void proposeUNResolution(CountryID country, const std::string& title, const std::string& desc);
    void voteUNResolution(CountryID country, uint32_t resolutionId, int vote); // 1=sim, -1=não, 0=abstém
    void sendForeignAid(CountryID from, CountryID to, Money amount);

    // Consultas
    DiplomaticRelation getRelation(CountryID a, CountryID b) const;
    double getRelationScore(CountryID a, CountryID b) const;
    std::vector<DiplomaticProposal> getPendingProposals(CountryID country) const;
    std::vector<CountryID> getAllies(CountryID country) const;
    std::vector<CountryID> getEnemies(CountryID country) const;
    std::vector<CountryID> getNeighbors(CountryID country) const;
    double calculateAcceptanceProbability(const DiplomaticProposal& proposal) const;
    bool isAtWar(CountryID a, CountryID b) const;
    bool isAllied(CountryID a, CountryID b) const;
    double getDiplomaticIsolation(CountryID country) const;

private:
    void updateRelations(double dt);
    void processProposals(const SimDate& date);
    void updateOrganizations(double dt);
    void checkForConflictEscalation(const SimDate& date);
    void calculateDiplomaticInfluence(Country& country);
    double evaluateProposal(const DiplomaticProposal& proposal) const;

    WorldState& world_;
    const SimulationConfig& config_;

    std::vector<DiplomaticProposal> pendingProposals_;
    std::vector<DiplomaticProposal> proposalHistory_;
    std::vector<UNResolution> unResolutions_;
    uint32_t nextProposalId_ = 1;
};

} // namespace GPS
