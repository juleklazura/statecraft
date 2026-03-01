/**
 * GPS - Geopolitical Simulator
 * EventSystem.cpp
 */

#include "systems/events/EventSystem.h"
#include <iostream>

namespace GPS {

EventSystem::EventSystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void EventSystem::init() {
    std::cout << "[Events] Initializing event system..." << std::endl;
    registerDefaultTriggers();
}

void EventSystem::update(double deltaTime, const SimDate& currentDate) {
    if (currentDate.hour == 0) {
        checkTriggers(currentDate);
        updateActiveEvents(deltaTime);
        resolveExpiredEvents();
    }
}

void EventSystem::shutdown() {
    std::cout << "[Events] Shutting down." << std::endl;
}

void EventSystem::registerDefaultTriggers() {
    // Crise financeira
    {
        EventTrigger trigger;
        trigger.name = "Financial Crisis";
        trigger.baseProbability = 0.0005;
        trigger.cooldownDays = 365 * 3;
        trigger.condition = [](const WorldState& world, CountryID id) {
            auto& c = world.getCountry(id);
            return c.debtToGDP > 0.8 || c.inflation > 0.15 || c.gdpGrowthRate < -0.02;
        };
        trigger.generator = [this](const WorldState& w, CountryID id, const SimDate& d) {
            return generateFinancialCrisis(id, d);
        };
        triggers_.push_back(trigger);
    }

    // Epidemia
    {
        EventTrigger trigger;
        trigger.name = "Epidemic";
        trigger.baseProbability = 0.0002;
        trigger.cooldownDays = 365 * 5;
        trigger.condition = [](const WorldState& world, CountryID id) {
            auto& c = world.getCountry(id);
            return c.budgetHealthcare < 0.10 || c.hdi < 0.5;
        };
        trigger.generator = [this](const WorldState& w, CountryID id, const SimDate& d) {
            return generateEpidemic(id, d);
        };
        triggers_.push_back(trigger);
    }

    // Crise política
    {
        EventTrigger trigger;
        trigger.name = "Political Crisis";
        trigger.baseProbability = 0.001;
        trigger.cooldownDays = 180;
        trigger.condition = [](const WorldState& world, CountryID id) {
            auto& c = world.getCountry(id);
            return c.governmentApproval < 0.25 || c.internalStability < 0.3;
        };
        trigger.generator = [this](const WorldState& w, CountryID id, const SimDate& d) {
            return generatePoliticalCrisis(id, d);
        };
        triggers_.push_back(trigger);
    }

    // Ataque terrorista
    {
        EventTrigger trigger;
        trigger.name = "Terror Attack";
        trigger.baseProbability = 0.0003;
        trigger.cooldownDays = 90;
        trigger.condition = [](const WorldState& world, CountryID id) {
            auto& c = world.getCountry(id);
            return c.terrorismThreat > 0.3;
        };
        trigger.generator = [this](const WorldState& w, CountryID id, const SimDate& d) {
            return generateTerrorAttack(id, d);
        };
        triggers_.push_back(trigger);
    }

    // Inquietação social
    {
        EventTrigger trigger;
        trigger.name = "Social Unrest";
        trigger.baseProbability = 0.002;
        trigger.cooldownDays = 60;
        trigger.condition = [](const WorldState& world, CountryID id) {
            auto& c = world.getCountry(id);
            return c.unemploymentRate > 0.15 || c.giniCoefficient > 0.5;
        };
        trigger.generator = [this](const WorldState& w, CountryID id, const SimDate& d) {
            return generateSocialUnrest(id, d);
        };
        triggers_.push_back(trigger);
    }

    // Ataque cibernético
    {
        EventTrigger trigger;
        trigger.name = "Cyber Attack";
        trigger.baseProbability = 0.001;
        trigger.cooldownDays = 90;
        trigger.condition = [](const WorldState& world, CountryID id) {
            auto& c = world.getCountry(id);
            return c.cyberDefense < 0.4;
        };
        trigger.generator = [this](const WorldState& w, CountryID id, const SimDate& d) {
            return generateCyberAttack(id, d);
        };
        triggers_.push_back(trigger);
    }
}

void EventSystem::checkTriggers(const SimDate& date) {
    int today = date.totalDays();

    for (auto& trigger : triggers_) {
        if (today - trigger.lastTriggeredDay < trigger.cooldownDays) continue;

        for (auto id : world_.getAllCountryIds()) {
            if (!trigger.condition(world_, id)) continue;

            double prob = trigger.baseProbability * config_.crisisFrequency;
            if (RandomEngine::instance().chance(prob)) {
                GameEvent event = trigger.generator(world_, id, date);
                event.id = nextEventId_++;
                events_[event.id] = event;
                trigger.lastTriggeredDay = today;
                break; // Um evento por trigger por check
            }
        }
    }
}

void EventSystem::updateActiveEvents(double dt) {
    for (auto& [id, event] : events_) {
        if (!event.active) continue;

        event.daysElapsed += static_cast<int>(dt);

        // Aplicar efeitos contínuos
        if (event.primaryCountry != INVALID_COUNTRY) {
            auto& country = world_.getCountry(event.primaryCountry);
            applyEventEffects(event, country);
        }

        if (event.daysElapsed >= event.durationDays) {
            event.active = false;
        }
    }
}

void EventSystem::resolveExpiredEvents() {
    // Eventos expirados são simplesmente desativados
}

void EventSystem::applyEventEffects(const GameEvent& event, Country& country) {
    double dailyFactor = 1.0 / std::max(1, event.durationDays);

    country.gdp.billions += event.economicImpact * dailyFactor;
    country.internalStability += event.stabilityImpact * dailyFactor;
    country.governmentApproval += event.diplomaticImpact * dailyFactor * 0.1;
}

// ===== Geradores de Eventos =====

GameEvent EventSystem::generateFinancialCrisis(CountryID country, const SimDate& date) {
    auto& c = world_.getCountry(country);
    GameEvent event;
    event.title = "Crise Financeira em " + c.name;
    event.description = "Os mercados financeiros entraram em colapso. A confiança dos investidores despencou.";
    event.date = date;
    event.crisisType = CrisisType::FINANCIAL_CRASH;
    event.primaryCountry = country;
    event.severity = RandomEngine::instance().randDouble(0.5, 0.9);
    event.durationDays = RandomEngine::instance().randInt(90, 365);
    event.economicImpact = -c.gdp.billions * 0.05 * event.severity;
    event.stabilityImpact = -0.15 * event.severity;

    GameEvent::ResponseOption bail{"Pacote de resgate bancário", c.gdp.billions * 0.03,
                                    -0.2, -0.1, 0.02, "Estabiliza mercados mas custa caro"};
    GameEvent::ResponseOption austerity{"Medidas de austeridade", 0, -0.1, -0.15, 0.01,
                                         "Reduz gastos mas afeta população"};
    GameEvent::ResponseOption stimulus{"Estímulo econômico massivo", c.gdp.billions * 0.05,
                                       -0.3, 0.05, -0.01, "Injeta dinheiro na economia"};
    event.responseOptions = {bail, austerity, stimulus};
    return event;
}

GameEvent EventSystem::generateEpidemic(CountryID country, const SimDate& date) {
    auto& c = world_.getCountry(country);
    GameEvent event;
    event.title = "Epidemia em " + c.name;
    event.description = "Uma doença infecciosa está se espalhando rapidamente.";
    event.date = date;
    event.crisisType = CrisisType::EPIDEMIC;
    event.primaryCountry = country;
    event.severity = RandomEngine::instance().randDouble(0.3, 0.8);
    event.durationDays = RandomEngine::instance().randInt(60, 180);
    event.economicImpact = -c.gdp.billions * 0.02 * event.severity;
    event.stabilityImpact = -0.1 * event.severity;
    event.populationImpact = -c.population * 0.001 * event.severity;

    GameEvent::ResponseOption quarantine{"Quarentena nacional", c.gdp.billions * 0.02,
                                          -0.4, -0.1, 0.01, "Controla doença mas paralisa economia"};
    GameEvent::ResponseOption partial{"Medidas parciais", c.gdp.billions * 0.01,
                                       -0.2, -0.05, 0, "Equilíbrio entre saúde e economia"};
    event.responseOptions = {quarantine, partial};
    return event;
}

GameEvent EventSystem::generatePoliticalCrisis(CountryID country, const SimDate& date) {
    auto& c = world_.getCountry(country);
    GameEvent event;
    event.title = "Crise Política em " + c.name;
    event.description = "Escândalo político abala o governo. Oposição exige renúncia.";
    event.date = date;
    event.crisisType = CrisisType::POLITICAL_CRISIS;
    event.primaryCountry = country;
    event.severity = RandomEngine::instance().randDouble(0.4, 0.8);
    event.durationDays = RandomEngine::instance().randInt(30, 120);
    event.stabilityImpact = -0.2 * event.severity;

    GameEvent::ResponseOption address{"Discurso à nação", 0, -0.1, 0.05, 0, "Tenta reconquistar confiança"};
    GameEvent::ResponseOption reshuffle{"Reformar gabinete", 0, -0.15, 0.0, 0, "Novas faces no governo"};
    event.responseOptions = {address, reshuffle};
    return event;
}

GameEvent EventSystem::generateTerrorAttack(CountryID country, const SimDate& date) {
    auto& c = world_.getCountry(country);
    GameEvent event;
    event.title = "Ataque Terrorista em " + c.name;
    event.description = "Um ataque terrorista chocou a nação.";
    event.date = date;
    event.crisisType = CrisisType::TERRORISM;
    event.primaryCountry = country;
    event.severity = RandomEngine::instance().randDouble(0.3, 0.7);
    event.durationDays = RandomEngine::instance().randInt(7, 30);
    event.stabilityImpact = -0.15 * event.severity;
    event.economicImpact = -c.gdp.billions * 0.005;
    return event;
}

GameEvent EventSystem::generateSocialUnrest(CountryID country, const SimDate& date) {
    auto& c = world_.getCountry(country);
    GameEvent event;
    event.title = "Inquietação Social em " + c.name;
    event.description = "Protestos em massa tomam as ruas das principais cidades.";
    event.date = date;
    event.crisisType = CrisisType::SOCIAL_UNREST;
    event.primaryCountry = country;
    event.severity = RandomEngine::instance().randDouble(0.3, 0.7);
    event.durationDays = RandomEngine::instance().randInt(14, 60);
    event.stabilityImpact = -0.1 * event.severity;
    event.economicImpact = -c.gdp.billions * 0.01;

    GameEvent::ResponseOption concessions{"Concessões aos manifestantes", c.gdp.billions * 0.01,
                                           -0.3, 0.1, -0.005, "Cede às demandas parcialmente"};
    GameEvent::ResponseOption force{"Repressão policial", 0, 0.1, -0.2, 0, "Força pode piorar situação"};
    event.responseOptions = {concessions, force};
    return event;
}

GameEvent EventSystem::generateCyberAttack(CountryID country, const SimDate& date) {
    auto& c = world_.getCountry(country);
    GameEvent event;
    event.title = "Ataque Cibernético em " + c.name;
    event.description = "Infraestrutura digital crítica foi comprometida.";
    event.date = date;
    event.crisisType = CrisisType::CYBER_ATTACK;
    event.primaryCountry = country;
    event.severity = RandomEngine::instance().randDouble(0.3, 0.6);
    event.durationDays = RandomEngine::instance().randInt(7, 30);
    event.economicImpact = -c.gdp.billions * 0.005;
    return event;
}

GameEvent EventSystem::generateDiplomaticIncident(CountryID country, const SimDate& date) {
    auto& c = world_.getCountry(country);
    GameEvent event;
    event.title = "Incidente Diplomático envolvendo " + c.name;
    event.description = "Uma crise diplomática ameaça as relações internacionais.";
    event.date = date;
    event.crisisType = CrisisType::DIPLOMATIC_INCIDENT;
    event.primaryCountry = country;
    event.severity = RandomEngine::instance().randDouble(0.2, 0.5);
    event.durationDays = RandomEngine::instance().randInt(14, 60);
    event.diplomaticImpact = -0.1 * event.severity;
    return event;
}

GameEvent EventSystem::generateNaturalDisaster(CountryID country, const SimDate& date) {
    return generateFinancialCrisis(country, date); // Placeholder - desastres naturais são do EnvironmentSystem
}

// ===== Ações do Jogador =====

void EventSystem::respondToEvent(EventID eventId, int responseIndex) {
    auto it = events_.find(eventId);
    if (it == events_.end()) return;

    auto& event = it->second;
    if (responseIndex < 0 || responseIndex >= static_cast<int>(event.responseOptions.size())) return;

    event.chosenResponse = responseIndex;
    auto& response = event.responseOptions[responseIndex];

    // Aplicar efeitos da resposta
    event.severity += response.effectOnSeverity;
    event.severity = std::clamp(event.severity, 0.0, 1.0);

    if (event.primaryCountry != INVALID_COUNTRY) {
        auto& country = world_.getCountry(event.primaryCountry);
        country.governmentApproval += response.effectOnApproval;
        country.gdp.billions += country.gdp.billions * response.effectOnEconomy;
        double cost = std::min(response.cost, country.foreignReserves.billions);
        country.foreignReserves -= Money(cost);
        if (country.foreignReserves.billions < 0.0) country.foreignReserves = Money(0.0);
    }
}

void EventSystem::dismissEvent(EventID eventId) {
    auto it = events_.find(eventId);
    if (it != events_.end()) {
        it->second.active = false;
    }
}

EventID EventSystem::forceEvent(CrisisType type, CountryID country, double severity) {
    SimDate now; // Placeholder
    GameEvent event;
    switch (type) {
        case CrisisType::FINANCIAL_CRASH:
            event = generateFinancialCrisis(country, now); break;
        case CrisisType::EPIDEMIC:
            event = generateEpidemic(country, now); break;
        case CrisisType::POLITICAL_CRISIS:
            event = generatePoliticalCrisis(country, now); break;
        default:
            event = generatePoliticalCrisis(country, now); break;
    }
    event.severity = severity;
    event.id = nextEventId_++;
    events_[event.id] = event;
    return event.id;
}

// ===== Consultas =====

std::vector<GameEvent> EventSystem::getActiveEvents() const {
    std::vector<GameEvent> result;
    for (const auto& [id, e] : events_) {
        if (e.active) result.push_back(e);
    }
    return result;
}

std::vector<GameEvent> EventSystem::getEventsForCountry(CountryID country) const {
    std::vector<GameEvent> result;
    for (const auto& [id, e] : events_) {
        if (e.primaryCountry == country) result.push_back(e);
    }
    return result;
}

std::vector<GameEvent> EventSystem::getPendingPlayerEvents() const {
    std::vector<GameEvent> result;
    CountryID player = world_.getPlayerCountry();
    for (const auto& [id, e] : events_) {
        if (e.active && e.primaryCountry == player && e.chosenResponse < 0) {
            result.push_back(e);
        }
    }
    return result;
}

GameEvent EventSystem::getEvent(EventID id) const {
    auto it = events_.find(id);
    if (it != events_.end()) return it->second;
    return {};
}

int EventSystem::getActiveEventCount() const {
    int count = 0;
    for (const auto& [id, e] : events_) {
        if (e.active) count++;
    }
    return count;
}

void EventSystem::registerTrigger(EventTrigger trigger) {
    triggers_.push_back(std::move(trigger));
}

void EventSystem::registerCustomEvent(const GameEvent& event, const SimDate& triggerDate) {
    auto e = event;
    e.id = nextEventId_++;
    e.date = triggerDate;
    events_[e.id] = e;
}

} // namespace GPS
