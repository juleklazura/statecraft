/**
 * GPS - Geopolitical Simulator
 * EventSystem.h - Sistema de eventos, crises e caos
 * 
 * Gera eventos não roteirizados baseados no estado da simulação.
 * Crises financeiras, epidemias, desastres, conflitos inesperados.
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct GameEvent {
    EventID id;
    std::string title;
    std::string description;
    SimDate date;
    CrisisType crisisType;
    CountryID primaryCountry = INVALID_COUNTRY;
    std::vector<CountryID> affectedCountries;
    double severity = 0.5;
    bool active = true;
    int durationDays = 30;
    int daysElapsed = 0;

    // Efeitos
    double economicImpact = 0.0;      // Negativo = dano
    double stabilityImpact = 0.0;
    double populationImpact = 0.0;
    double militaryImpact = 0.0;
    double diplomaticImpact = 0.0;
    double environmentalImpact = 0.0;

    // Opções de resposta do jogador
    struct ResponseOption {
        std::string text;
        double cost = 0.0;
        double effectOnSeverity = 0.0;   // Negativo = reduz severidade
        double effectOnApproval = 0.0;
        double effectOnEconomy = 0.0;
        std::string consequence;
    };
    std::vector<ResponseOption> responseOptions;
    int chosenResponse = -1;
};

struct EventTrigger {
    std::string name;
    std::function<bool(const WorldState&, CountryID)> condition;
    std::function<GameEvent(const WorldState&, CountryID, const SimDate&)> generator;
    double baseProbability = 0.01;  // Chance diária quando condição é verdadeira
    int cooldownDays = 365;
    int lastTriggeredDay = -9999;
};

class EventSystem : public ISystem {
public:
    explicit EventSystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "EventSystem"; }
    int getPriority() const override { return 70; }

    // Ações do jogador
    void respondToEvent(EventID eventId, int responseIndex);
    void dismissEvent(EventID eventId);

    // Consultas
    std::vector<GameEvent> getActiveEvents() const;
    std::vector<GameEvent> getEventsForCountry(CountryID country) const;
    std::vector<GameEvent> getPendingPlayerEvents() const;
    GameEvent getEvent(EventID id) const;
    int getActiveEventCount() const;

    // Modding
    void registerTrigger(EventTrigger trigger);
    void registerCustomEvent(const GameEvent& event, const SimDate& triggerDate);

    // Geração forçada (para debug/modding)
    EventID forceEvent(CrisisType type, CountryID country, double severity);

private:
    void checkTriggers(const SimDate& date);
    void updateActiveEvents(double dt);
    void resolveExpiredEvents();
    void registerDefaultTriggers();
    void applyEventEffects(const GameEvent& event, Country& country);

    // Geradores de eventos específicos
    GameEvent generateFinancialCrisis(CountryID country, const SimDate& date);
    GameEvent generateEpidemic(CountryID country, const SimDate& date);
    GameEvent generateNaturalDisaster(CountryID country, const SimDate& date);
    GameEvent generatePoliticalCrisis(CountryID country, const SimDate& date);
    GameEvent generateTerrorAttack(CountryID country, const SimDate& date);
    GameEvent generateSocialUnrest(CountryID country, const SimDate& date);
    GameEvent generateCyberAttack(CountryID country, const SimDate& date);
    GameEvent generateDiplomaticIncident(CountryID country, const SimDate& date);

    WorldState& world_;
    const SimulationConfig& config_;

    std::unordered_map<EventID, GameEvent> events_;
    std::vector<EventTrigger> triggers_;
    EventID nextEventId_ = 1;
};

} // namespace GPS
