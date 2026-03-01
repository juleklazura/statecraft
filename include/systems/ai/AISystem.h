/**
 * GPS - Geopolitical Simulator
 * AISystem.h - Inteligência artificial dos países controlados por IA
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

// Forward declarations
class EconomySystem;
class PoliticsSystem;
class DiplomacySystem;
class MilitarySystem;
class LawSystem;

struct AIPersonality {
    double aggression = 0.5;       // Tendência belicista
    double diplomacy = 0.5;        // Tendência diplomática
    double economicFocus = 0.5;    // Foco econômico
    double militarism = 0.5;       // Militarismo
    double expansionism = 0.3;     // Tendência expansionista
    double cooperation = 0.5;      // Cooperação internacional
    double riskTolerance = 0.5;    // Tolerância a riscos
    double pragmatism = 0.5;       // Pragmatismo vs ideologia
    double nationalism = 0.5;      // Nacionalismo
    double authoritarianism = 0.5; // Tendência autoritária

    // Prioridades (devem somar 1.0)
    double priorityEconomy = 0.25;
    double priorityMilitary = 0.20;
    double priorityDiplomacy = 0.20;
    double priorityInternal = 0.20;
    double priorityScience = 0.15;
};

struct AIDecision {
    CountryID country;
    std::string description;
    SimDate date;

    enum class Type {
        ECONOMIC_POLICY,
        MILITARY_ACTION,
        DIPLOMATIC_MOVE,
        INTERNAL_POLICY,
        CRISIS_RESPONSE,
        LAW_PROPOSAL,
        ELECTION_STRATEGY,
        INTELLIGENCE_OP
    } type;

    double urgency = 0.5;
    double expectedBenefit = 0.0;
    double risk = 0.0;
};

class AISystem : public ISystem {
public:
    explicit AISystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "AISystem"; }
    int getPriority() const override { return 80; }

    // Configuração
    void setPersonality(CountryID country, const AIPersonality& personality);
    AIPersonality getPersonality(CountryID country) const;
    void setAIActive(CountryID country, bool active);
    bool isAIActive(CountryID country) const;

    // Ligação de sistemas (chamado na inicialização)
    void linkSystems(EconomySystem* eco, PoliticsSystem* pol,
                     DiplomacySystem* dip, MilitarySystem* mil,
                     LawSystem* law);

    // Consultas
    std::vector<AIDecision> getRecentDecisions(CountryID country, int maxDays = 30) const;

private:
    void updateCountryAI(Country& country, const SimDate& date, double dt);
    void makeEconomicDecisions(Country& country, const AIPersonality& personality);
    void makeMilitaryDecisions(Country& country, const AIPersonality& personality);
    void makeDiplomaticDecisions(Country& country, const AIPersonality& personality);
    void makeInternalDecisions(Country& country, const AIPersonality& personality);
    void respondToCrisis(Country& country, const AIPersonality& personality);
    void evaluateThreats(Country& country);
    void evaluateOpportunities(Country& country);
    double assessCountryRisk(CountryID country) const;
    double assessMilitaryBalance(CountryID us, CountryID them) const;

    WorldState& world_;
    const SimulationConfig& config_;

    std::unordered_map<CountryID, AIPersonality> personalities_;
    std::unordered_map<CountryID, std::vector<AIDecision>> decisionHistory_;

    // Referências a outros sistemas
    EconomySystem* economySystem_ = nullptr;
    PoliticsSystem* politicsSystem_ = nullptr;
    DiplomacySystem* diplomacySystem_ = nullptr;
    MilitarySystem* militarySystem_ = nullptr;
    LawSystem* lawSystem_ = nullptr;
};

} // namespace GPS
