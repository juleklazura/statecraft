/**
 * GPS - Geopolitical Simulator
 * MilitarySystem.h - Sistema de defesa, forças armadas e conflitos
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct MilitaryUnit {
    UnitID id;
    CountryID country;
    MilitaryBranch branch;
    std::string name;

    double strength = 1.0;      // 0-1, integridade
    double morale = 0.7;
    double experience = 0.3;
    double supply = 1.0;        // 0-1, nível de suprimentos

    int personnel = 0;
    int heavyEquipment = 0;

    GeoCoord position;
    RegionID stationedAt = INVALID_REGION;
    bool deployed = false;
    bool inCombat = false;
};

struct MilitaryConflict {
    uint32_t id;
    std::string name;
    std::vector<CountryID> attackers;
    std::vector<CountryID> defenders;
    SimDate startDate;
    bool active = true;

    // Estatísticas
    std::unordered_map<CountryID, double> casualties;
    std::unordered_map<CountryID, double> equipmentLosses;
    double civilianCasualties = 0.0;
    Money totalCost;

    // Progresso (-1 a 1, positivo = atacantes vencendo)
    double frontProgress = 0.0;

    // Escalada
    double escalationLevel = 0.0;  // 0-1
    bool nuclearThreat = false;

    enum class Type {
        BORDER_SKIRMISH,
        LIMITED_WAR,
        FULL_SCALE_WAR,
        CIVIL_WAR,
        PROXY_WAR,
        INSURGENCY,
        INVASION,
        LIBERATION,
        PEACEKEEPING
    } type;
};

struct MilitaryOperation {
    OperationID id;
    CountryID country;
    std::string name;
    std::vector<UnitID> units;
    GeoCoord target;

    enum class Type {
        DEPLOYMENT,
        WITHDRAWAL,
        OFFENSIVE,
        DEFENSIVE,
        PATROL,
        BLOCKADE,
        AIRSTRIKE,
        MISSILE_STRIKE,
        SPECIAL_OPS,
        EVACUATION,
        HUMANITARIAN
    } type;

    double progress = 0.0;
    bool active = true;
    SimDate startDate;
};

class MilitarySystem : public ISystem {
public:
    explicit MilitarySystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "MilitarySystem"; }
    int getPriority() const override { return 40; }

    // Ações do jogador
    UnitID createUnit(CountryID country, MilitaryBranch branch, const std::string& name, int personnel);
    void disbandUnit(UnitID unit);
    void deployUnit(UnitID unit, GeoCoord destination);
    void recallUnit(UnitID unit);
    OperationID launchOperation(CountryID country, MilitaryOperation::Type type,
                                 std::vector<UnitID> units, GeoCoord target, const std::string& name);
    void cancelOperation(OperationID op);
    void setMilitaryBudget(CountryID country, double gdpShare);
    void conscript(CountryID country, int number);
    void demobilize(CountryID country, int number);
    void purchaseEquipment(CountryID country, MilitaryBranch branch, int quantity, Money cost);
    void buildNuclearWeapon(CountryID country); // Leva tempo, custa muito, muda diplomacia

    // Conflitos
    uint32_t startConflict(const std::string& name, std::vector<CountryID> attackers,
                           std::vector<CountryID> defenders, MilitaryConflict::Type type);
    void joinConflict(uint32_t conflictId, CountryID country, bool asAttacker);
    void endConflict(uint32_t conflictId);

    // Consultas
    MilitaryForces getForces(CountryID country) const;
    std::vector<MilitaryUnit> getUnits(CountryID country) const;
    std::vector<MilitaryConflict> getActiveConflicts() const;
    std::vector<MilitaryConflict> getConflictsOf(CountryID country) const;
    double getMilitaryPower(CountryID country) const;
    double getWarWeariness(CountryID country) const;
    double getNuclearDeterrence(CountryID country) const;
    bool isAtWar(CountryID country) const;
    double getConflictCost(CountryID country) const;

private:
    void updateConflicts(double dt, const SimDate& date);
    void updateUnits(double dt);
    void updateOperations(double dt);
    void processAttrition(MilitaryConflict& conflict, double dt);
    void calculateBattleOutcome(MilitaryConflict& conflict);
    void checkEscalation(MilitaryConflict& conflict);
    double calculateUnitPower(const MilitaryUnit& unit) const;

    WorldState& world_;
    const SimulationConfig& config_;

    std::unordered_map<UnitID, MilitaryUnit> units_;
    std::vector<MilitaryConflict> conflicts_;
    std::unordered_map<OperationID, MilitaryOperation> operations_;
    std::unordered_map<CountryID, double> warWeariness_;

    UnitID nextUnitId_ = 1;
    uint32_t nextConflictId_ = 1;
    OperationID nextOpId_ = 1;
};

} // namespace GPS
