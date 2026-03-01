/**
 * GPS - Geopolitical Simulator
 * LawSystem.h - Sistema legislativo e de leis
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct Law {
    LawID id;
    std::string name;
    std::string description;
    LawCategory category;
    CountryID country;

    // Status legislativo
    enum class Status {
        PROPOSED,
        IN_COMMITTEE,
        DEBATING,
        VOTING,
        APPROVED,
        VETOED,
        ACTIVE,
        REPEALED,
        EXPIRED
    } status = Status::PROPOSED;

    SimDate proposedDate;
    SimDate enactedDate;
    int implementationDays = 30;   // Dias para implementar totalmente
    int implementationProgress = 0;

    // Apoio político
    double parliamentarySupport = 0.5;
    double publicSupport = 0.5;
    double judicialApproval = 0.8;  // Chance de ser aprovada pelo judiciário

    // Efeitos de curto prazo (imediatos)
    struct Effects {
        double gdpGrowth = 0.0;
        double inflation = 0.0;
        double unemployment = 0.0;
        double taxRevenue = 0.0;
        double governmentSpending = 0.0;
        double inequality = 0.0;
        double corruption = 0.0;
        double criminalRate = 0.0;
        double pollution = 0.0;
        double education = 0.0;
        double healthcare = 0.0;
        double happiness = 0.0;
        double stability = 0.0;
        double immigration = 0.0;
        double freedom = 0.0;
        double militaryPower = 0.0;
    };

    Effects shortTermEffects;   // Nos primeiros 6 meses
    Effects longTermEffects;    // Após 2+ anos

    // Impacto em grupos sociais (positivo = aprovam, negativo = desaprovam)
    std::unordered_map<SocialGroupType, double> groupImpact;

    // Custo de implementação
    Money implementationCost;
    Money yearlyMaintenanceCost;

    // Pré-requisitos
    std::vector<LawID> prerequisiteLaws;
    std::vector<LawID> conflictingLaws; // Revogadas automaticamente se esta for aprovada
};

class LawSystem : public ISystem {
public:
    explicit LawSystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "LawSystem"; }
    int getPriority() const override { return 25; }

    // Ações do jogador
    LawID proposeLaw(CountryID country, const Law& law);
    bool pushLawThroughParliament(LawID id);  // Forçar votação
    void revokeLaw(CountryID country, LawID id);
    void amendLaw(LawID id, const Law::Effects& newEffects);

    // Consultas
    Law getLaw(LawID id) const;
    std::vector<Law> getActiveLaws(CountryID country) const;
    std::vector<Law> getPendingLaws(CountryID country) const;
    std::vector<Law> getLawsByCategory(CountryID country, LawCategory category) const;
    double calculateLawSupport(const Law& law, CountryID country) const;
    bool canProposeLaw(CountryID country, const Law& law) const;
    // Retorna templates da categoria, filtrando leis já ativas/em tramitação para o país
    std::vector<Law> getAvailableLawTemplates(LawCategory category) const;
    std::vector<Law> getAvailableLawTemplates(LawCategory category, CountryID country) const;
    bool isLawAlreadyActive(CountryID country, const std::string& lawName) const;
    // Retorna "" se pode propor, ou mensagem explicando o bloqueio
    std::string getLawBlockReason(CountryID country, const std::string& lawName) const;

    // Modding
    void registerLawTemplate(const Law& templateLaw);

private:
    void processLegislation(Country& country, double dt, const SimDate& date);
    void implementLaw(Law& law, Country& country, double dt);
    void applyLawEffects(const Law& law, Country& country, double progress);
    void checkJudicialReview(Law& law, const Country& country);

    WorldState& world_;
    const SimulationConfig& config_;

    std::unordered_map<LawID, Law> laws_;
    std::vector<Law> lawTemplates_;
    LawID nextLawId_ = 1;
};

} // namespace GPS
