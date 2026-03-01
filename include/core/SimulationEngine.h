/**
 * GPS - Geopolitical Simulator
 * SimulationEngine.h - Motor principal de simulação
 * 
 * O motor central coordena todos os subsistemas, gerencia o tempo de
 * simulação, e garante a causalidade sistêmica entre todos os módulos.
 */
#pragma once

#include "core/Types.h"
#include <memory>
#include <vector>
#include <functional>

namespace GPS {

// Forward declarations
class WorldState;
class EconomySystem;
class PoliticsSystem;
class PopulationSystem;
class DiplomacySystem;
class MilitarySystem;
class IntelligenceSystem;
class EnvironmentSystem;
class InfrastructureSystem;
class EventSystem;
class LawSystem;
class ElectoralSystem;
class AISystem;
class ModdingSystem;

/**
 * ISystem - Interface base para todos os subsistemas
 * Todo subsistema deve implementar init(), update() e shutdown()
 */
class ISystem {
public:
    virtual ~ISystem() = default;

    virtual void init() = 0;
    virtual void update(double deltaTime, const SimDate& currentDate) = 0;
    virtual void shutdown() = 0;

    virtual const char* getName() const = 0;
    virtual int getPriority() const { return 100; } // Menor = executa primeiro
};

/**
 * SimulationConfig - Configuração global da simulação
 */
struct SimulationConfig {
    Difficulty difficulty = Difficulty::NORMAL;
    GameSpeed speed = GameSpeed::NORMAL;
    uint64_t randomSeed = 0; // 0 = aleatório

    // Ajustes de realismo
    double economicVolatility = 1.0;    // Multiplicador de volatilidade econômica
    double aiAggression = 1.0;          // Agressividade da IA
    double crisisFrequency = 1.0;       // Frequência de crises
    double publicOpinionSensitivity = 1.0;
    double militaryEscalation = 1.0;

    // Flags de módulos
    bool enableNuclearWeapons = true;
    bool enableClimateChange = true;
    bool enableEspionage = true;
    bool enableModding = true;
    bool enableDetailedEconomy = true;
    bool enableElections = true;

    // Performance
    int maxCountriesSimulated = 195;
    bool simplifyDistantAI = true;      // IA simplificada para países distantes
};

/**
 * SimulationEngine - Motor central do simulador
 * 
 * Gerencia o ciclo de vida de todos os subsistemas e coordena
 * a simulação global em tempo contínuo.
 */
class SimulationEngine {
public:
    SimulationEngine();
    ~SimulationEngine();

    // Lifecycle
    bool initialize(const SimulationConfig& config);
    void shutdown();

    // Game Loop
    void update(double realDeltaTime);
    void forceAdvance(int hours);  // Avança N horas ignorando o estado de pausa

    // Controle de tempo
    void setSpeed(GameSpeed speed);
    GameSpeed getSpeed() const { return config_.speed; }
    void pause() { config_.speed = GameSpeed::PAUSED; }
    void resume(GameSpeed speed = GameSpeed::NORMAL) { config_.speed = speed; }
    bool isPaused() const { return config_.speed == GameSpeed::PAUSED; }

    // Acesso ao estado do mundo
    WorldState& getWorld() { return *world_; }
    const WorldState& getWorld() const { return *world_; }

    // Acesso aos sistemas
    EconomySystem& getEconomy() { return *economySystem_; }
    PoliticsSystem& getPolitics() { return *politicsSystem_; }
    PopulationSystem& getPopulation() { return *populationSystem_; }
    DiplomacySystem& getDiplomacy() { return *diplomacySystem_; }
    MilitarySystem& getMilitary() { return *militarySystem_; }
    IntelligenceSystem& getIntelligence() { return *intelligenceSystem_; }
    EnvironmentSystem& getEnvironment() { return *environmentSystem_; }
    InfrastructureSystem& getInfrastructure() { return *infrastructureSystem_; }
    EventSystem& getEvents() { return *eventSystem_; }
    LawSystem& getLaws() { return *lawSystem_; }
    ElectoralSystem& getElectoral() { return *electoralSystem_; }
    AISystem& getAI() { return *aiSystem_; }

    // Config
    const SimulationConfig& getConfig() const { return config_; }
    SimDate getCurrentDate() const { return currentDate_; }

    // Callbacks
    using DateCallback = std::function<void(const SimDate&)>;
    void onNewDay(DateCallback cb) { dayCallbacks_.push_back(std::move(cb)); }
    void onNewMonth(DateCallback cb) { monthCallbacks_.push_back(std::move(cb)); }
    void onNewYear(DateCallback cb) { yearCallbacks_.push_back(std::move(cb)); }

    // Estatísticas
    double getSimulationFPS() const { return simFPS_; }
    uint64_t getTotalTicksProcessed() const { return totalTicks_; }

private:
    void tickSimulation();
    void processHour();
    void processDay();
    void processMonth();
    void processYear();
    double getSpeedMultiplier() const;

    SimulationConfig config_;
    SimDate currentDate_;
    double accumulatedTime_ = 0.0;
    double simFPS_ = 0.0;
    uint64_t totalTicks_ = 0;
    bool initialized_ = false;

    // Subsistemas (ordem importa para dependências)
    std::unique_ptr<WorldState> world_;
    std::unique_ptr<EconomySystem> economySystem_;
    std::unique_ptr<PoliticsSystem> politicsSystem_;
    std::unique_ptr<PopulationSystem> populationSystem_;
    std::unique_ptr<DiplomacySystem> diplomacySystem_;
    std::unique_ptr<MilitarySystem> militarySystem_;
    std::unique_ptr<IntelligenceSystem> intelligenceSystem_;
    std::unique_ptr<EnvironmentSystem> environmentSystem_;
    std::unique_ptr<InfrastructureSystem> infrastructureSystem_;
    std::unique_ptr<EventSystem> eventSystem_;
    std::unique_ptr<LawSystem> lawSystem_;
    std::unique_ptr<ElectoralSystem> electoralSystem_;
    std::unique_ptr<AISystem> aiSystem_;
    std::unique_ptr<ModdingSystem> moddingSystem_;

    // Vetor ordenado por prioridade
    std::vector<ISystem*> systemsOrdered_;

    // Callbacks de eventos temporais
    std::vector<DateCallback> dayCallbacks_;
    std::vector<DateCallback> monthCallbacks_;
    std::vector<DateCallback> yearCallbacks_;
};

} // namespace GPS
