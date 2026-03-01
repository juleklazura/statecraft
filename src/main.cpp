/**
 * GPS - Geopolitical Simulator
 * main.cpp - Ponto de entrada do jogo
 *
 * Inicializa o motor de simulação com dados demo e
 * executa a interface FTXUI.
 */

#include "core/SimulationEngine.h"
#include "world/WorldState.h"
#include "core/Types.h"
#include "ui/GameUI.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/ioctl.h>

using namespace GPS;

// ----------------------------------------------------------------
// Resolução mínima exigida (colunas × linhas)
// ----------------------------------------------------------------
static constexpr int MIN_COLS = 160;
static constexpr int MIN_ROWS = 44;

// ====================================================================
// Criação do mundo de demonstração com 8 países realistas
// ====================================================================
void createDemoWorld(WorldState& world) {
    // ============== Brasil ==============
    {
        Country c;
        c.id = 1;
        c.name = "Brasil";
        c.officialName = "República Federativa do Brasil";
        c.isoCode = "BRA";
        c.capital = "Brasília";
        c.capitalCoord = {-15.79f, -47.88f};
        c.areaSqKm = 8515767;
        c.coastlineKm = 7491;
        c.government = GovernmentType::PRESIDENTIAL_REPUBLIC;
        c.economicSystem = EconomicSystem::MIXED_ECONOMY;
        c.headOfState = "Presidente do Brasil";
        c.population = 215.0;
        c.populationGrowthRate = 0.005;
        c.lifeExpectancy = 76.0;
        c.medianAge = 34.0;
        c.urbanizationRate = 0.87;
        c.literacyRate = 0.93;
        c.hdi = 0.754;
        c.gdp = Money(1.92e3);
        c.gdpGrowthRate = 0.029;
        c.gdpPerCapita = 8917.0;
        c.inflation = 0.046;
        c.unemploymentRate = 0.08;
        c.giniCoefficient = 0.53;
        c.interestRate = 0.1075;
        c.exchangeRate = 4.97;
        c.publicDebt = Money(1.52e3);
        c.debtToGDP = 0.79;
        c.foreignReserves = Money(330.0);
        c.governmentApproval = 0.38;
        c.corruption = 0.43;
        c.ruleOfLaw = 0.48;
        c.democracy = 0.69;
        c.pressLiberty = 0.57;
        c.internalStability = 0.60;
        c.budgetDefense = 0.04;
        c.budgetEducation = 0.16;
        c.budgetHealthcare = 0.10;
        c.budgetSocialWelfare = 0.30;
        c.budgetInfrastructure = 0.06;
        c.co2EmissionsMT = 476.0;
        c.energyProductionTWh = 690.0;
        c.energyConsumptionTWh = 580.0;
        c.renewableShare = 0.83;
        c.softPower = 0.45;
        c.military.activePersonnel = 366500;
        c.military.reservePersonnel = 1340000;
        c.military.nuclearWarheads = 0;
        c.incomeTaxLow = 0.195;   // IRPF 7.5% + INSS ~12% (empregado+empregador parcial)
        c.incomeTaxMid = 0.275;   // IRPF 15.0% + INSS ~12.5%
        c.incomeTaxHigh = 0.375;  // IRPF 27.5% + INSS capped ~10%
        c.corporateTax = 0.34;    // IRPJ 25% + CSLL 9%
        c.salesTax = 0.27;        // ICMS ~17% + PIS/COFINS ~6% + ISS ~2% + IPI ~2%
        c.importTariff = 0.03;    // Tarifa média brasileira ~3%
        c.ruleOfLaw = 0.52;       // Capacidade de arrecadar razoável
        c.neighbors = {2, 5};
        c.isPlayerControlled = true;
        c.isAIActive = false;
        world.addCountry(c);
    }

    // ============== Argentina ==============
    {
        Country c;
        c.id = 2;
        c.name = "Argentina";
        c.officialName = "República Argentina";
        c.isoCode = "ARG";
        c.capital = "Buenos Aires";
        c.capitalCoord = {-34.60f, -58.38f};
        c.areaSqKm = 2780400;
        c.coastlineKm = 4989;
        c.government = GovernmentType::PRESIDENTIAL_REPUBLIC;
        c.economicSystem = EconomicSystem::MIXED_ECONOMY;
        c.headOfState = "Presidente da Argentina";
        c.population = 46.0;
        c.populationGrowthRate = 0.009;
        c.lifeExpectancy = 77.0;
        c.medianAge = 32.0;
        c.urbanizationRate = 0.92;
        c.literacyRate = 0.99;
        c.hdi = 0.842;
        c.gdp = Money(640.0);
        c.gdpGrowthRate = -0.015;
        c.gdpPerCapita = 13650.0;
        c.inflation = 0.95;
        c.unemploymentRate = 0.07;
        c.giniCoefficient = 0.42;
        c.interestRate = 0.97;
        c.exchangeRate = 350.0;
        c.publicDebt = Money(400.0);
        c.debtToGDP = 0.62;
        c.foreignReserves = Money(25.0);
        c.governmentApproval = 0.42;
        c.corruption = 0.50;
        c.democracy = 0.72;
        c.internalStability = 0.50;
        c.softPower = 0.35;
        c.military.activePersonnel = 80000;
        c.co2EmissionsMT = 170.0;
        c.neighbors = {1};
        world.addCountry(c);
    }

    // ============== EUA ==============
    {
        Country c;
        c.id = 3;
        c.name = "Estados Unidos";
        c.officialName = "Estados Unidos da América";
        c.isoCode = "USA";
        c.capital = "Washington D.C.";
        c.capitalCoord = {38.90f, -77.04f};
        c.areaSqKm = 9833520;
        c.coastlineKm = 19924;
        c.government = GovernmentType::PRESIDENTIAL_REPUBLIC;
        c.economicSystem = EconomicSystem::FREE_MARKET;
        c.headOfState = "Presidente dos EUA";
        c.population = 333.0;
        c.populationGrowthRate = 0.004;
        c.lifeExpectancy = 78.0;
        c.medianAge = 38.0;
        c.urbanizationRate = 0.83;
        c.literacyRate = 0.99;
        c.hdi = 0.921;
        c.gdp = Money(25.5e3);
        c.gdpGrowthRate = 0.021;
        c.gdpPerCapita = 76399.0;
        c.inflation = 0.034;
        c.unemploymentRate = 0.037;
        c.giniCoefficient = 0.41;
        c.interestRate = 0.055;
        c.exchangeRate = 1.0;
        c.publicDebt = Money(33.0e3);
        c.debtToGDP = 1.29;
        c.foreignReserves = Money(240.0);
        c.governmentApproval = 0.40;
        c.corruption = 0.33;
        c.democracy = 0.79;
        c.internalStability = 0.65;
        c.softPower = 0.85;
        c.military.activePersonnel = 1395350;
        c.military.reservePersonnel = 845000;
        c.military.nuclearWarheads = 5550;
        c.budgetDefense = 0.13;
        c.co2EmissionsMT = 5007.0;
        c.energyProductionTWh = 4380.0;
        c.energyConsumptionTWh = 3930.0;
        c.renewableShare = 0.21;
        c.neighbors = {4};
        world.addCountry(c);
    }

    // ============== México ==============
    {
        Country c;
        c.id = 4;
        c.name = "México";
        c.officialName = "Estados Unidos Mexicanos";
        c.isoCode = "MEX";
        c.capital = "Cidade do México";
        c.government = GovernmentType::PRESIDENTIAL_REPUBLIC;
        c.economicSystem = EconomicSystem::MIXED_ECONOMY;
        c.population = 130.0;
        c.hdi = 0.758;
        c.gdp = Money(1.32e3);
        c.gdpGrowthRate = 0.032;
        c.inflation = 0.055;
        c.unemploymentRate = 0.035;
        c.giniCoefficient = 0.45;
        c.democracy = 0.60;
        c.internalStability = 0.45;
        c.corruption = 0.60;
        c.softPower = 0.35;
        c.military.activePersonnel = 277000;
        c.co2EmissionsMT = 470.0;
        c.neighbors = {3};
        world.addCountry(c);
    }

    // ============== Colômbia ==============
    {
        Country c;
        c.id = 5;
        c.name = "Colômbia";
        c.isoCode = "COL";
        c.capital = "Bogotá";
        c.government = GovernmentType::PRESIDENTIAL_REPUBLIC;
        c.economicSystem = EconomicSystem::MIXED_ECONOMY;
        c.population = 52.0;
        c.hdi = 0.752;
        c.gdp = Money(340.0);
        c.gdpGrowthRate = 0.075;
        c.inflation = 0.095;
        c.unemploymentRate = 0.10;
        c.democracy = 0.64;
        c.internalStability = 0.50;
        c.corruption = 0.55;
        c.softPower = 0.25;
        c.military.activePersonnel = 293200;
        c.co2EmissionsMT = 92.0;
        c.neighbors = {1};
        c.terrorismThreat = 0.3;
        world.addCountry(c);
    }

    // ============== China ==============
    {
        Country c;
        c.id = 6;
        c.name = "China";
        c.officialName = "República Popular da China";
        c.isoCode = "CHN";
        c.capital = "Pequim";
        c.government = GovernmentType::ONE_PARTY_STATE;
        c.economicSystem = EconomicSystem::STATE_CAPITALISM;
        c.population = 1412.0;
        c.populationGrowthRate = -0.001;
        c.lifeExpectancy = 78.0;
        c.medianAge = 39.0;
        c.hdi = 0.768;
        c.gdp = Money(17.9e3);
        c.gdpGrowthRate = 0.052;
        c.gdpPerCapita = 12720.0;
        c.inflation = 0.007;
        c.unemploymentRate = 0.052;
        c.giniCoefficient = 0.38;
        c.interestRate = 0.035;
        c.exchangeRate = 7.1;
        c.publicDebt = Money(14.0e3);
        c.debtToGDP = 0.78;
        c.foreignReserves = Money(3200.0);
        c.democracy = 0.10;
        c.pressLiberty = 0.10;
        c.internalStability = 0.75;
        c.corruption = 0.45;
        c.softPower = 0.55;
        c.military.activePersonnel = 2035000;
        c.military.reservePersonnel = 510000;
        c.military.nuclearWarheads = 350;
        c.budgetDefense = 0.07;
        c.co2EmissionsMT = 11472.0;
        c.energyProductionTWh = 8530.0;
        c.energyConsumptionTWh = 8270.0;
        c.renewableShare = 0.30;
        c.neighbors = {7};
        world.addCountry(c);
    }

    // ============== Rússia ==============
    {
        Country c;
        c.id = 7;
        c.name = "Rússia";
        c.officialName = "Federação Russa";
        c.isoCode = "RUS";
        c.capital = "Moscou";
        c.government = GovernmentType::SEMI_PRESIDENTIAL;
        c.economicSystem = EconomicSystem::STATE_CAPITALISM;
        c.population = 144.0;
        c.populationGrowthRate = -0.005;
        c.lifeExpectancy = 73.0;
        c.medianAge = 40.0;
        c.hdi = 0.822;
        c.gdp = Money(1.78e3);
        c.gdpGrowthRate = -0.02;
        c.inflation = 0.12;
        c.unemploymentRate = 0.03;
        c.giniCoefficient = 0.36;
        c.interestRate = 0.16;
        c.exchangeRate = 90.0;
        c.publicDebt = Money(305.0);
        c.debtToGDP = 0.17;
        c.foreignReserves = Money(580.0);
        c.democracy = 0.20;
        c.pressLiberty = 0.12;
        c.internalStability = 0.60;
        c.corruption = 0.60;
        c.softPower = 0.30;
        c.military.activePersonnel = 1150000;
        c.military.reservePersonnel = 2000000;
        c.military.nuclearWarheads = 6257;
        c.budgetDefense = 0.12;
        c.co2EmissionsMT = 1756.0;
        c.energyProductionTWh = 1190.0;
        c.oilProductionBarrelsDay = 10.5e6;
        c.neighbors = {6};
        world.addCountry(c);
    }

    // ============== Alemanha ==============
    {
        Country c;
        c.id = 8;
        c.name = "Alemanha";
        c.officialName = "República Federal da Alemanha";
        c.isoCode = "DEU";
        c.capital = "Berlim";
        c.government = GovernmentType::PARLIAMENTARY_REPUBLIC;
        c.economicSystem = EconomicSystem::SOCIAL_MARKET;
        c.population = 84.0;
        c.populationGrowthRate = 0.001;
        c.lifeExpectancy = 81.0;
        c.medianAge = 46.0;
        c.hdi = 0.942;
        c.gdp = Money(4.26e3);
        c.gdpGrowthRate = 0.001;
        c.gdpPerCapita = 50790.0;
        c.inflation = 0.032;
        c.unemploymentRate = 0.055;
        c.giniCoefficient = 0.31;
        c.interestRate = 0.045;
        c.exchangeRate = 0.92;
        c.publicDebt = Money(2.80e3);
        c.debtToGDP = 0.66;
        c.foreignReserves = Money(270.0);
        c.democracy = 0.90;
        c.pressLiberty = 0.85;
        c.internalStability = 0.80;
        c.corruption = 0.20;
        c.softPower = 0.70;
        c.military.activePersonnel = 183500;
        c.military.nuclearWarheads = 0;
        c.budgetDefense = 0.05;
        c.co2EmissionsMT = 674.0;
        c.renewableShare = 0.46;
        world.addCountry(c);
    }

    // ============== Relações diplomáticas iniciais ==============
    {
        DiplomaticRelation rel;
        rel.relations = 60;
        rel.status = DiplomaticStatus::FRIENDLY;
        rel.hasTradeAgreement = true;
        world.setRelation(1, 2, rel);
    }
    {
        DiplomaticRelation rel;
        rel.relations = 40;
        rel.status = DiplomaticStatus::FRIENDLY;
        world.setRelation(1, 3, rel);
    }
    {
        DiplomaticRelation rel;
        rel.relations = 30;
        rel.status = DiplomaticStatus::NEUTRAL;
        rel.hasTradeAgreement = true;
        world.setRelation(1, 6, rel);
    }
    {
        DiplomaticRelation rel;
        rel.relations = -30;
        rel.status = DiplomaticStatus::HOSTILE;
        world.setRelation(3, 6, rel);
    }
    {
        DiplomaticRelation rel;
        rel.relations = -60;
        rel.status = DiplomaticStatus::HOSTILE;
        world.setRelation(3, 7, rel);
    }
    {
        DiplomaticRelation rel;
        rel.relations = 50;
        rel.status = DiplomaticStatus::FRIENDLY;
        rel.hasTradeAgreement = true;
        world.setRelation(6, 7, rel);
    }
    {
        DiplomaticRelation rel;
        rel.relations = 70;
        rel.status = DiplomaticStatus::ALLIED;
        rel.hasTradeAgreement = true;
        world.setRelation(3, 8, rel);
    }

    world.setPlayerCountry(1);
}

// ====================================================================
// Programa principal
// ====================================================================
int main(int argc, char* argv[]) {

    // ----------------------------------------------------------------
    // Se não foi lançado com --dedicated, reabrir em janela própria
    // com tamanho garantido de MIN_COLS × MIN_ROWS.
    // ----------------------------------------------------------------
    bool dedicated = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--dedicated") == 0) {
            dedicated = true;
            break;
        }
    }

    if (!dedicated) {
        // Caminho do executável atual
        char exePath[4096] = {};
        ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
        if (len < 0) {
            // Fallback: usar argv[0]
            std::strncpy(exePath, argv[0], sizeof(exePath) - 1);
        }

        // Tenta gnome-terminal (disponível neste sistema)
        // gnome-terminal --geometry=COLSxROWS -- <programa> --dedicated
        std::string geom = std::to_string(MIN_COLS) + "x" + std::to_string(MIN_ROWS);
        std::string cmd =
            std::string("gnome-terminal")
            + " --geometry=" + geom
            + " --title=\"GPS - Geopolitical Simulator\""
            + " -- \"" + exePath + "\" --dedicated &";

        int ret = std::system(cmd.c_str());
        if (ret != 0) {
            // Fallback para xterm se gnome-terminal falhar
            cmd = std::string("xterm")
                + " -geometry " + geom
                + " -T \"GPS - Geopolitical Simulator\""
                + " -fa 'Monospace' -fs 11"
                + " -e \"" + exePath + "\" --dedicated &";
            std::system(cmd.c_str());
        }
        // Encerra o processo atual (a janela nova foi aberta)
        return 0;
    }

    // ----------------------------------------------------------------
    // Verificar tamanho mínimo do terminal
    // ----------------------------------------------------------------
    {
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
            if (ws.ws_col < MIN_COLS || ws.ws_row < MIN_ROWS) {
                std::cerr << "\n"
                    << "  ╔══════════════════════════════════════════════════════╗\n"
                    << "  ║          GPS - Geopolitical Simulator                ║\n"
                    << "  ╠══════════════════════════════════════════════════════╣\n"
                    << "  ║  ⚠  RESOLUÇÃO INSUFICIENTE                           ║\n"
                    << "  ║                                                      ║\n"
                    << "  ║  Tamanho atual : " << ws.ws_col << " × " << ws.ws_row << " (colunas × linhas)"
                    << std::string(std::max(0, 35 - (int)std::to_string(ws.ws_col).size()
                                                   - (int)std::to_string(ws.ws_row).size()), ' ')
                    << "║\n"
                    << "  ║  Mínimo exigido: " << MIN_COLS << " × " << MIN_ROWS << " (colunas × linhas)         ║\n"
                    << "  ║                                                      ║\n"
                    << "  ║  Redimensione a janela e tente novamente.            ║\n"
                    << "  ╚══════════════════════════════════════════════════════╝\n"
                    << "\n";
                std::cout << "Pressione ENTER para sair...";
                std::cin.get();
                return 1;
            }
        }
    }

    SimulationConfig config;
    config.difficulty = Difficulty::NORMAL;
    config.speed = GameSpeed::PAUSED;  // UI controla o tempo manualmente
    config.randomSeed = 42;
    config.enableModding = false;

    SimulationEngine engine;
    if (!engine.initialize(config)) {
        std::cerr << "ERRO: Falha ao inicializar o motor de simulação!\n";
        return 1;
    }

    createDemoWorld(engine.getWorld());

    std::cout << "[GPS] " << engine.getWorld().getCountryCount() << " países carregados.\n";
    std::cout << "[GPS] Controlando: " << engine.getWorld().getCountry(
        engine.getWorld().getPlayerCountry()).name << "\n";
    std::cout << "[GPS] Iniciando interface...\n";

    GameUI ui(engine);
    ui.run();

    engine.shutdown();
    return 0;
}
