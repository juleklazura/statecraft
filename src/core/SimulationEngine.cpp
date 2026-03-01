/**
 * GPS - Geopolitical Simulator
 * SimulationEngine.cpp - Implementação do motor principal de simulação
 */

#include "core/SimulationEngine.h"
#include "world/WorldState.h"
#include "systems/economy/EconomySystem.h"
#include "systems/politics/PoliticsSystem.h"
#include "systems/population/PopulationSystem.h"
#include "systems/diplomacy/DiplomacySystem.h"
#include "systems/military/MilitarySystem.h"
#include "systems/intelligence/IntelligenceSystem.h"
#include "systems/environment/EnvironmentSystem.h"
#include "systems/infrastructure/InfrastructureSystem.h"
#include "systems/events/EventSystem.h"
#include "systems/laws/LawSystem.h"
#include "systems/electoral/ElectoralSystem.h"
#include "systems/ai/AISystem.h"
#include "systems/modding/ModdingSystem.h"

#include <algorithm>
#include <iostream>
#include <chrono>
#include <cmath>

namespace GPS {

SimulationEngine::SimulationEngine() = default;
SimulationEngine::~SimulationEngine() { shutdown(); }

bool SimulationEngine::initialize(const SimulationConfig& config) {
    if (initialized_) {
        std::cerr << "[GPS] Engine already initialized!" << std::endl;
        return false;
    }

    config_ = config;
    currentDate_ = {2026, 1, 1, 0};

    // Seed do RNG
    if (config_.randomSeed != 0) {
        RandomEngine::instance().seed(config_.randomSeed);
    }

    std::cout << "[GPS] Initializing Geopolitical Simulator v0.1.0..." << std::endl;

    // Criar mundo
    world_ = std::make_unique<WorldState>();

    // Criar subsistemas na ordem correta de dependência
    economySystem_ = std::make_unique<EconomySystem>(*world_, config_);
    lawSystem_ = std::make_unique<LawSystem>(*world_, config_);
    politicsSystem_ = std::make_unique<PoliticsSystem>(*world_, config_);
    populationSystem_ = std::make_unique<PopulationSystem>(*world_, config_);
    electoralSystem_ = std::make_unique<ElectoralSystem>(*world_, config_);
    diplomacySystem_ = std::make_unique<DiplomacySystem>(*world_, config_);
    militarySystem_ = std::make_unique<MilitarySystem>(*world_, config_);
    intelligenceSystem_ = std::make_unique<IntelligenceSystem>(*world_, config_);
    environmentSystem_ = std::make_unique<EnvironmentSystem>(*world_, config_);
    infrastructureSystem_ = std::make_unique<InfrastructureSystem>(*world_, config_);
    eventSystem_ = std::make_unique<EventSystem>(*world_, config_);
    aiSystem_ = std::make_unique<AISystem>(*world_, config_);
    moddingSystem_ = std::make_unique<ModdingSystem>(*world_, config_);

    // Linkar IA aos outros sistemas
    aiSystem_->linkSystems(
        economySystem_.get(),
        politicsSystem_.get(),
        diplomacySystem_.get(),
        militarySystem_.get(),
        lawSystem_.get()
    );

    // Construir vetor ordenado por prioridade
    systemsOrdered_ = {
        economySystem_.get(),
        populationSystem_.get(),
        politicsSystem_.get(),
        electoralSystem_.get(),
        lawSystem_.get(),
        diplomacySystem_.get(),
        militarySystem_.get(),
        intelligenceSystem_.get(),
        infrastructureSystem_.get(),
        environmentSystem_.get(),
        eventSystem_.get(),
        aiSystem_.get(),
        moddingSystem_.get()
    };

    std::sort(systemsOrdered_.begin(), systemsOrdered_.end(),
              [](const ISystem* a, const ISystem* b) {
                  return a->getPriority() < b->getPriority();
              });

    // Inicializar todos os sistemas
    for (auto* sys : systemsOrdered_) {
        std::cout << "[GPS] Initializing " << sys->getName()
                  << " (priority: " << sys->getPriority() << ")" << std::endl;
        sys->init();
    }

    initialized_ = true;
    std::cout << "[GPS] Engine initialized successfully!" << std::endl;
    std::cout << "[GPS] Start date: " << currentDate_.toString() << std::endl;
    return true;
}

void SimulationEngine::shutdown() {
    if (!initialized_) return;

    std::cout << "[GPS] Shutting down engine..." << std::endl;

    // Shutdown na ordem reversa
    for (auto it = systemsOrdered_.rbegin(); it != systemsOrdered_.rend(); ++it) {
        (*it)->shutdown();
    }

    systemsOrdered_.clear();
    moddingSystem_.reset();
    aiSystem_.reset();
    eventSystem_.reset();
    infrastructureSystem_.reset();
    environmentSystem_.reset();
    intelligenceSystem_.reset();
    militarySystem_.reset();
    diplomacySystem_.reset();
    electoralSystem_.reset();
    populationSystem_.reset();
    politicsSystem_.reset();
    lawSystem_.reset();
    economySystem_.reset();
    world_.reset();

    initialized_ = false;
    std::cout << "[GPS] Engine shut down." << std::endl;
}

void SimulationEngine::update(double realDeltaTime) {
    if (!initialized_ || isPaused()) return;

    double speedMult = getSpeedMultiplier();
    accumulatedTime_ += realDeltaTime * speedMult;

    // Cada tick = 1 hora de simulação
    const double HOUR_STEP = 1.0; // 1 unidade = 1 hora

    auto startTime = std::chrono::high_resolution_clock::now();
    int ticksThisFrame = 0;
    const int MAX_TICKS_PER_FRAME = 168; // Max 1 semana por frame

    while (accumulatedTime_ >= HOUR_STEP && ticksThisFrame < MAX_TICKS_PER_FRAME) {
        accumulatedTime_ -= HOUR_STEP;
        tickSimulation();
        ticksThisFrame++;
    }

    // Limitar acúmulo para evitar espiral de morte
    if (accumulatedTime_ > HOUR_STEP * 48) {
        accumulatedTime_ = HOUR_STEP * 48;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double frameTime = std::chrono::duration<double>(endTime - startTime).count();
    if (frameTime > 0) {
        simFPS_ = ticksThisFrame / frameTime;
    }
}

void SimulationEngine::forceAdvance(int hours) {
    if (!initialized_) return;
    for (int i = 0; i < hours; ++i) {
        tickSimulation();
    }
}

void SimulationEngine::tickSimulation() {
    uint8_t prevDay = currentDate_.day;
    uint8_t prevMonth = currentDate_.month;
    uint16_t prevYear = currentDate_.year;

    processHour();
    currentDate_.advanceHour();
    totalTicks_++;

    // Verificar transições de período
    if (currentDate_.day != prevDay) {
        processDay();
        for (auto& cb : dayCallbacks_) cb(currentDate_);
    }
    if (currentDate_.month != prevMonth) {
        processMonth();
        for (auto& cb : monthCallbacks_) cb(currentDate_);
    }
    if (currentDate_.year != prevYear) {
        processYear();
        for (auto& cb : yearCallbacks_) cb(currentDate_);
    }
}

void SimulationEngine::processHour() {
    // Update horário - só sistemas que precisam de granularidade fina
    // (mercados, conflitos ativos, etc.)
    double dt = 1.0 / 24.0; // Fração do dia

    // Atualizar apenas sistemas com necessidade de alta frequência
    economySystem_->update(dt, currentDate_);
    militarySystem_->update(dt, currentDate_);
}

void SimulationEngine::processDay() {
    double dt = 1.0; // 1 dia

    // Atualizar todos os sistemas
    for (auto* sys : systemsOrdered_) {
        sys->update(dt, currentDate_);
    }

    // Decair modificadores
    for (auto id : world_->getAllCountryIds()) {
        // Modificadores são decaídos nos subsistemas individuais
        (void)id;
    }
}

void SimulationEngine::processMonth() {
#ifdef GPS_DEBUG_SIM
    std::cout << "[GPS] Month: " << currentDate_.toString() << std::endl;
#endif

    for (auto id : world_->getAllCountryIds()) {
        auto& c  = world_->getCountry(id);
        auto  ba = economySystem_->getBudgetAllocation(id);

        // ── Carga tributária real (atualiza mensalmente) ─────────────────
        if (c.gdp.billions > 0.0)
            c.taxBurden = std::clamp(c.governmentRevenue.billions / c.gdp.billions, 0.05, 0.70);

        // ── IDH (Índice de Desenvolvimento Humano) ──────────────────────
        // Componente renda: escala log(gdpPerCapita/100) / log(75000/100)
        double incomeIdx = std::clamp(
            std::log(std::max(100.0, c.gdpPerCapita) / 100.0) /
            std::log(75000.0 / 100.0), 0.0, 1.0);
        // Componente saúde: (lifeExp - 20) / (85 - 20)
        double healthIdx = std::clamp((c.lifeExpectancy - 20.0) / 65.0, 0.0, 1.0);
        // Componente educação: alfabetização + investimento
        double eduIdx = std::clamp(
            c.literacyRate * 0.55 + ba.education * 2.5 + ba.science * 0.5, 0.0, 1.0);
        double targetHDI = (incomeIdx + healthIdx + eduIdx) / 3.0;
        c.hdi += (targetHDI - c.hdi) * 0.05;  // 5% convergência/mês
        c.hdi = std::clamp(c.hdi, 0.10, 1.0);

        // ── Expectativa de vida ──────────────────────────────────────────
        double targetLife = std::clamp(
            45.0
            + ba.healthcare * 200.0          // gasto em saúde (proporção orçamento)
            + std::sqrt(std::max(0.0, c.gdpPerCapita)) * 0.08
            - c.giniCoefficient * 10.0       // desigualdade reduz expectativa
            - c.unemploymentRate * 5.0,
            40.0, 95.0);
        c.lifeExpectancy += (targetLife - c.lifeExpectancy) * 0.03;
        c.lifeExpectancy = std::clamp(c.lifeExpectancy, 40.0, 95.0);

        // ── Taxa de alfabetização ────────────────────────────────────────
        double targetLiteracy = std::clamp(
            0.25 + ba.education * 3.5 + c.hdi * 0.35, 0.20, 1.0);
        c.literacyRate += (targetLiteracy - c.literacyRate) * 0.03;
        c.literacyRate = std::clamp(c.literacyRate, 0.10, 1.0);

        // ── Estado de Direito (Rule of Law) ──────────────────────────────
        double targetRoL = std::clamp(
            c.democracy * 0.30
            + (1.0 - c.corruption) * 0.20
            + ba.security * 3.0
            + ba.administration * 2.5
            + c.institutionalTrust * 0.15,
            0.0, 1.0);
        c.ruleOfLaw += (targetRoL - c.ruleOfLaw) * 0.04;
        c.ruleOfLaw = std::clamp(c.ruleOfLaw, 0.0, 1.0);

        // ── Corrupção ────────────────────────────────────────────────────
        // Reduz com mais adm., democracia, liberdade de imprensa, estado de direito
        double targetCorruption = std::clamp(
            0.95
            - c.democracy * 0.30
            - c.pressLiberty * 0.20
            - ba.administration * 3.5
            - c.ruleOfLaw * 0.25,
            0.0, 1.0);
        c.corruption += (targetCorruption - c.corruption) * 0.03;
        c.corruption = std::clamp(c.corruption, 0.0, 1.0);

        // ── Confiança Institucional ──────────────────────────────────────
        double targetTrust = std::clamp(
            c.ruleOfLaw * 0.35
            + (1.0 - c.corruption) * 0.30
            + c.democracy * 0.20
            + c.governmentApproval * 0.15,
            0.0, 1.0);
        c.institutionalTrust += (targetTrust - c.institutionalTrust) * 0.05;
        c.institutionalTrust = std::clamp(c.institutionalTrust, 0.0, 1.0);

        // ── Segurança de Fronteira ────────────────────────────────────────
        double targetBorder = std::clamp(
            ba.security * 4.0 + ba.defense * 2.0 + c.ruleOfLaw * 0.3,
            0.0, 1.0);
        c.borderSecurity += (targetBorder - c.borderSecurity) * 0.06;
        c.borderSecurity = std::clamp(c.borderSecurity, 0.0, 1.0);

        // ── Defesa Cibernética ───────────────────────────────────────────
        double targetCyber = std::clamp(
            ba.science * 5.0 + ba.administration * 2.0 + c.hdi * 0.3,
            0.0, 1.0);
        c.cyberDefense += (targetCyber - c.cyberDefense) * 0.06;
        c.cyberDefense = std::clamp(c.cyberDefense, 0.0, 1.0);

        // ── Ameaça Terrorista ─────────────────────────────────────────────
        double targetTerror = std::clamp(
            c.terrorismThreat
            + (1.0 - c.internalStability) * 0.02
            - ba.security * 0.15
            - c.ruleOfLaw * 0.05
            - c.borderSecurity * 0.03,
            0.0, 1.0);
        c.terrorismThreat += (targetTerror - c.terrorismThreat) * 0.06;
        c.terrorismThreat = std::clamp(c.terrorismThreat, 0.0, 1.0);

        // ── Soft Power ────────────────────────────────────────────────────
        double targetSoft = std::clamp(
            ba.culture * 4.0 + c.hdi * 0.35 + c.democracy * 0.20
            + ba.education * 0.5 + c.pressLiberty * 0.10,
            0.0, 1.0);
        c.softPower += (targetSoft - c.softPower) * 0.03;
        c.softPower = std::clamp(c.softPower, 0.0, 1.0);

        // ── Urbanização (crescimento lento com PIB) ───────────────────────
        if (c.gdpGrowthRate > 0.0 && c.urbanizationRate < 0.98) {
            c.urbanizationRate += ba.infrastructure * 0.002 + ba.transport * 0.001;
            c.urbanizationRate = std::clamp(c.urbanizationRate, 0.0, 0.98);
        }

        // ── Qualidade do Ar (melhora com renováveis e investimento ambiental) ─
        double airTarget = std::clamp(
            180.0 - c.renewableShare * 150.0 - ba.environment * 200.0
            + c.co2EmissionsMT / (c.population + 1.0) * 0.5,
            5.0, 250.0);
        c.airQualityIndex += (airTarget - c.airQualityIndex) * 0.05;
        c.airQualityIndex = std::clamp(c.airQualityIndex, 5.0, 250.0);

        // ── Energia renovável (cresce com investimento ambiental + ciência) ───
        if (ba.environment + ba.science > 0.05) {
            c.renewableShare += (ba.environment * 0.02 + ba.science * 0.01);
            c.renewableShare = std::clamp(c.renewableShare, 0.0, 1.0);
        }
    }
}

void SimulationEngine::processYear() {
    // Processos anuais: crescimento do PIB, demografia, eleições
    std::cout << "[GPS] New Year: " << currentDate_.year << std::endl;
}

void SimulationEngine::setSpeed(GameSpeed speed) {
    config_.speed = speed;
}

double SimulationEngine::getSpeedMultiplier() const {
    switch (config_.speed) {
        case GameSpeed::PAUSED:    return 0.0;
        case GameSpeed::SLOW:      return 1.0;     // 1 hora/s
        case GameSpeed::NORMAL:    return 6.0;     // 6 horas/s
        case GameSpeed::FAST:      return 24.0;    // 1 dia/s
        case GameSpeed::VERY_FAST: return 168.0;   // 1 semana/s
        case GameSpeed::ULTRA_FAST: return 720.0;  // 1 mês/s
        default: return 1.0;
    }
}

} // namespace GPS
