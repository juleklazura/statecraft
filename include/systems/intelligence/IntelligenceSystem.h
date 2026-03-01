/**
 * GPS - Geopolitical Simulator
 * IntelligenceSystem.h - Serviços de inteligência, espionagem e operações secretas
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct IntelligenceAgency {
    CountryID country;
    std::string name;
    double budget = 0.0;         // Bilhões
    double capability = 0.5;     // 0-1
    double counterIntel = 0.5;   // Contra-inteligência
    double cyberCapability = 0.3;
    double humanIntel = 0.5;     // HUMINT
    double signalIntel = 0.3;    // SIGINT
    double imageIntel = 0.2;     // IMINT
    int operativeCount = 0;
    double secrecy = 0.7;        // Chance de operações não serem descobertas
};

struct CovertOperation {
    OperationID id;
    CountryID operator_;
    CountryID target;
    std::string codeName;
    SimDate startDate;
    int daysToComplete = 30;
    int daysElapsed = 0;
    double successProbability = 0.5;
    double discoveryRisk = 0.3;
    bool completed = false;
    bool discovered = false;
    bool successful = false;

    enum class Type {
        ESPIONAGE,            // Roubo de informações
        SABOTAGE,             // Destruição de infraestrutura
        ASSASSINATION,        // Eliminação de alvo
        PROPAGANDA,           // Influência de opinião pública
        ELECTION_INTERFERENCE, // Interferência eleitoral
        COUP_SUPPORT,         // Apoio a golpe
        REBEL_FUNDING,        // Financiamento de rebeldes
        CYBER_ATTACK,         // Ataque cibernético
        COUNTER_INTELLIGENCE, // Contra-espionagem
        DISINFORMATION,       // Desinformação
        BLACKMAIL,            // Chantagem
        ECONOMIC_WARFARE      // Guerra econômica secreta
    } type;

    // Resultados potenciais
    double economicDamage = 0.0;
    double stabilityDamage = 0.0;
    double diplomaticDamage = 0.0;  // Se descoberto
    std::string intelGathered;
};

struct IntelReport {
    CountryID about;
    SimDate date;
    std::string content;
    double reliability = 0.7;   // 0-1

    enum class Category {
        MILITARY_MOVEMENT,
        ECONOMIC_DATA,
        POLITICAL_INSTABILITY,
        NUCLEAR_PROGRAM,
        ALLIANCE_SHIFT,
        PLANNED_ATTACK,
        INTERNAL_DISSENT,
        TECHNOLOGY_DEVELOPMENT,
        ESPIONAGE_DETECTED
    } category;
};

class IntelligenceSystem : public ISystem {
public:
    explicit IntelligenceSystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "IntelligenceSystem"; }
    int getPriority() const override { return 50; }

    // Ações do jogador
    OperationID launchOperation(CountryID country, CovertOperation::Type type,
                                 CountryID target, const std::string& codeName);
    void cancelOperation(OperationID id);
    void setIntelBudget(CountryID country, double budgetBillions);
    void increaseCounterIntelligence(CountryID country);
    void recruitOperatives(CountryID country, int count);

    // Consultas
    IntelligenceAgency getAgency(CountryID country) const;
    std::vector<CovertOperation> getActiveOperations(CountryID country) const;
    std::vector<IntelReport> getRecentReports(CountryID country, int maxDays = 30) const;
    double getIntelligenceRating(CountryID country) const;
    std::vector<CovertOperation> getDiscoveredOperationsAgainst(CountryID country) const;
    bool hasIntelOn(CountryID observer, CountryID target) const;

private:
    void updateOperations(double dt, const SimDate& date);
    void generateIntelReports(const SimDate& date);
    void checkForDiscovery(CovertOperation& op);
    void resolveOperation(CovertOperation& op);
    void applyOperationEffects(const CovertOperation& op);
    void handleDiscovery(const CovertOperation& op);

    WorldState& world_;
    const SimulationConfig& config_;

    std::unordered_map<CountryID, IntelligenceAgency> agencies_;
    std::unordered_map<OperationID, CovertOperation> operations_;
    std::unordered_map<CountryID, std::vector<IntelReport>> reports_;
    OperationID nextOpId_ = 1;
};

} // namespace GPS
