/**
 * GPS - Geopolitical Simulator
 * ElectoralSystem.h - Sistema eleitoral e de campanhas
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct ElectionResult {
    CountryID country;
    SimDate date;
    std::map<PartyID, double> voteShare;
    std::map<PartyID, int> seatsWon;
    PartyID winner;
    double turnout = 0.65;
    double fraudLevel = 0.0;
    bool contested = false;
    std::string type; // "presidential", "parliamentary", "referendum"
};

struct ElectionCampaign {
    CountryID country;
    PartyID party;
    SimDate startDate;
    SimDate electionDate;
    Money budget;
    Money spent;

    double popularity = 0.5;
    double momentum = 0.0;
    double mediaCoverage = 0.5;
    double grassrootsSupport = 0.5;

    // Estratégias de campanha
    double advertisingSpend = 0.3;
    double rallyEffort = 0.2;
    double socialMediaPresence = 0.3;
    double dirtyCampaigning = 0.0;   // Campanha suja
    double promisesIntensity = 0.5;
};

struct PollData {
    SimDate date;
    CountryID country;
    std::map<PartyID, double> partySupport;
    double marginOfError = 0.03;
    double sampleSize = 1000;
};

struct Referendum {
    uint32_t id;
    CountryID country;
    std::string question;
    SimDate date;
    double yesVote = 0.0;
    double noVote = 0.0;
    double turnout = 0.0;
    bool completed = false;
    double threshold = 0.5;  // % necessário para aprovação
};

class ElectoralSystem : public ISystem {
public:
    explicit ElectoralSystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "ElectoralSystem"; }
    int getPriority() const override { return 22; }

    // Ações do jogador
    void callElection(CountryID country, const std::string& type, const SimDate& date);
    void setCampaignStrategy(CountryID country, PartyID party, const ElectionCampaign& campaign);
    void allocateCampaignFunds(CountryID country, PartyID party, Money amount);
    void callReferendum(CountryID country, const std::string& question, const SimDate& date);

    // Consultas
    bool isElectionScheduled(CountryID country) const;
    SimDate getNextElectionDate(CountryID country) const;
    std::vector<ElectionResult> getElectionHistory(CountryID country) const;
    PollData getLatestPoll(CountryID country) const;
    std::vector<PollData> getPollHistory(CountryID country) const;
    ElectionCampaign getCampaign(CountryID country, PartyID party) const;
    double getElectionFraudRisk(CountryID country) const;
    double predictElectionOutcome(CountryID country, PartyID party) const;

private:
    void processElection(CountryID country, const SimDate& date);
    void processReferendum(Referendum& ref, const SimDate& date);
    void updateCampaigns(double dt);
    void generatePolls(const SimDate& date);
    void updatePartyPopularity(Country& country, double dt);
    void applyElectionResults(CountryID country, const ElectionResult& result);

    WorldState& world_;
    const SimulationConfig& config_;

    std::unordered_map<CountryID, SimDate> scheduledElections_;
    std::unordered_map<CountryID, std::string> electionTypes_;
    std::unordered_map<CountryID, std::vector<ElectionResult>> electionHistory_;
    std::unordered_map<CountryID, std::vector<PollData>> pollHistory_;
    std::unordered_map<CountryID, std::map<PartyID, ElectionCampaign>> campaigns_;
    std::vector<Referendum> referendums_;
    uint32_t nextRefId_ = 1;
};

} // namespace GPS
