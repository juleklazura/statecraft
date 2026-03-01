/**
 * GPS - Geopolitical Simulator
 * EconomySystem.cpp - Implementação do sistema econômico
 */

#include "systems/economy/EconomySystem.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace GPS {

EconomySystem::EconomySystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void EconomySystem::init() {
    std::cout << "[Economy] Initializing economic simulation..." << std::endl;

    // Inicializar políticas fiscais e orçamentos padrão para todos os países
    for (auto id : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(id);

        TaxPolicy tax;
        tax.incomeTaxLow = country.incomeTaxLow;
        tax.incomeTaxMid = country.incomeTaxMid;
        tax.incomeTaxHigh = country.incomeTaxHigh;
        tax.corporateTax = country.corporateTax;
        tax.salesTax = country.salesTax;
        tax.importTariff = country.importTariff;
        tax.exportTariff = country.exportTariff;
        tax.carbonTax = country.carbonTax;
        taxPolicies_[id] = tax;

        BudgetAllocation budget;
        budget.defense = country.budgetDefense;
        budget.education = country.budgetEducation;
        budget.healthcare = country.budgetHealthcare;
        budget.infrastructure = country.budgetInfrastructure;
        budget.socialWelfare = country.budgetSocialWelfare;
        budget.science = country.budgetScience;
        budget.administration = country.budgetAdministration;
        budget.debtService = country.budgetDebtService;
        budget.security = country.budgetSecurity;
        budget.agriculture = country.budgetAgriculture;
        budget.culture = country.budgetCulture;
        budget.transport = country.budgetTransport;
        // Usa os mesmos multiplicadores da fórmula runtime (bases 0.65 e 0.68)
        double taxEff0 = std::clamp(
            0.50 + country.ruleOfLaw * 0.35 - country.corruption * 0.15,
            0.40, 0.97
        );
        double weightedIncome = country.incomeTaxLow  * 0.45
                              + country.incomeTaxMid  * 0.33
                              + country.incomeTaxHigh * 0.22;
        double estRevPct =
            0.65 * weightedIncome         * taxEff0 +  // IR + Contribuições Sociais
            0.12 * country.corporateTax   * taxEff0 +  // Corporativo
            0.68 * country.salesTax       * taxEff0 +  // Todos impostos sobre consumo
            0.22 * country.importTariff   * taxEff0 +  // Tarifas
            0.030                         * taxEff0 +  // Propriedade (default 0.01 × 3)
            0.012                         * taxEff0 +  // Ganhos de capital (0.15 × 0.08)
            0.02  * country.ruleOfLaw;                 // Receita não tributária
        // Países com dívida alta começam com déficit estrutural;
        // países com baixa dívida começam equilibrados ou superavitários
        double debtBias = country.debtToGDP * 0.025;
        budget.totalGdpRatio = std::clamp(estRevPct + debtBias, 0.10, 0.55);
        budgets_[id] = budget;

        MarketIndicators market;
        market.stockMarketIndex = 1000.0 + country.gdp.billions * 0.1;
        market.investorConfidence = country.ruleOfLaw * 0.5 + country.democracy * 0.3 + 0.2;
        market.creditRating = std::clamp(1.0 - country.debtToGDP * 0.5, 0.0, 1.0);
        markets_[id] = market;

        // Inicializar ciclo econômico
        if (country.gdpGrowthRate > 0.03)  economicCycles_[id] = EconomicCyclePhase::EXPANSION;
        else if (country.gdpGrowthRate > 0.0) economicCycles_[id] = EconomicCyclePhase::CONTRACTION;
        else economicCycles_[id] = EconomicCyclePhase::RECESSION;
        recessionQuarters_[id] = 0;

        // Indicadores de IED
        FDIIndicators fdi;
        fdi.stockBillions = country.gdp.billions * 0.30;
        fdi.inflowsBillions = country.gdp.billions * 0.03 * country.ruleOfLaw;
        fdi.fdiToGdpRatio = fdi.stockBillions / std::max(country.gdp.billions, 1.0);
        fdi.easeOfDoingBusiness = std::clamp(
            0.3 + country.ruleOfLaw * 0.4 - country.corruption * 0.3, 0.05, 0.95);
        fdi.politicalRiskPremium = (1.0 - country.internalStability) * 300.0;
        fdiData_[id] = fdi;

        // Saúde bancária inicial
        BankingIndicators banking;
        banking.creditToGdp = std::clamp(0.30 + country.ruleOfLaw * 0.60, 0.10, 1.50);
        banking.nonPerformingLoanRatio = std::clamp(
            0.03 + country.corruption * 0.08 - country.ruleOfLaw * 0.03, 0.01, 0.25);
        banking.capitalAdequacyRatio = std::clamp(
            0.10 + country.ruleOfLaw * 0.08, 0.04, 0.25);
        banking.centralBankIndependence = std::clamp(
            0.20 + country.democracy * 0.60 + country.ruleOfLaw * 0.20, 0.05, 1.0);
        banking.loanToDepositRatio = 0.80 + (banking.creditToGdp - 0.70) * 0.1;
        bankingData_[id] = banking;

        // Composição setorial (aproximação baseada no PIB per capita)
        SectoralBreakdown sector;
        double gdpPC = country.gdpPerCapita;
        // Países mais pobres = mais agricultura; mais ricos = mais serviços/tecnologia
        if (gdpPC < 5000.0) {
            sector.agriculture = 0.18; sector.industry = 0.28; sector.services = 0.44;
            sector.technology = 0.03; sector.energy = 0.04; sector.finance = 0.03;
        } else if (gdpPC < 15000.0) {
            sector.agriculture = 0.09; sector.industry = 0.30; sector.services = 0.49;
            sector.technology = 0.05; sector.energy = 0.04; sector.finance = 0.03;
        } else if (gdpPC < 40000.0) {
            sector.agriculture = 0.05; sector.industry = 0.25; sector.services = 0.55;
            sector.technology = 0.08; sector.energy = 0.04; sector.finance = 0.03;
        } else {
            sector.agriculture = 0.02; sector.industry = 0.18; sector.services = 0.58;
            sector.technology = 0.13; sector.energy = 0.04; sector.finance = 0.05;
        }
        sectors_[id] = sector;

        // Economia informal
        ShadowEconomy shadow;
        shadow.ratioToGdp = std::clamp(
            0.50 - country.ruleOfLaw * 0.30 - country.gdpPerCapita / 100000.0, 0.05, 0.60);
        shadow.informelEmploymentRatio = shadow.ratioToGdp * 1.8;
        shadow.taxGapBillions = country.gdp.billions * shadow.ratioToGdp *
            (country.incomeTaxMid + country.salesTax) * 0.5;
        shadowEconomies_[id] = shadow;

        // Distribuição de renda
        IncomeDistribution income;
        income.povertyRate = std::clamp(
            0.60 - country.gdpPerCapita / 50000.0 - country.budgetSocialWelfare * 0.5, 0.01, 0.80);
        income.extremePovertyRate = income.povertyRate * 0.25;
        income.incomeShareTop10 = std::clamp(
            0.20 + country.giniCoefficient * 0.5, 0.25, 0.65);
        income.incomeShareBottom50 = std::clamp(
            0.30 - country.giniCoefficient * 0.3, 0.05, 0.35);
        income.middleClassShare = 1.0 - income.povertyRate - income.incomeShareTop10 * 0.1;
        income.socialMobilityIndex = std::clamp(
            0.20 + country.ruleOfLaw * 0.30 + country.budgetEducation * 0.5 - country.corruption * 0.20,
            0.05, 0.95);
        incomeDistributions_[id] = income;

        // Fundo soberano (começa zerado)
        sovereignFunds_[id] = SovereignWealthData{};

        // Produtividade e confiança inicial
        productivityGrowth_[id] = 0.01;
        businessConfidence_[id] = std::clamp(
            0.30 + country.ruleOfLaw * 0.30 + country.democracy * 0.20 - country.corruption * 0.20,
            0.05, 0.95);
    }
}

void EconomySystem::update(double deltaTime, const SimDate& currentDate) {
    for (auto id : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(id);
        updateCountryEconomy(country, deltaTime);
        updateBankingSector(country, deltaTime);
        updateFDIFlows(country, deltaTime);
        updateSectoralBreakdown(country, deltaTime);
        updateShadowEconomy(country, deltaTime);
        updateIncomeDistribution(country, deltaTime);
        updateSovereignWealthFund(country, deltaTime);
        updateEconomicCycle(country, deltaTime);
    }

    // Atualizações globais diárias
    if (currentDate.hour == 0) {
        updateGlobalTrade(deltaTime);
        updateMarkets(deltaTime);

        // Atualizar índice de commodities (petróleo + metais + alimentos)
        globalCommodityIndex_ *= (1.0 + RandomEngine::instance().randNormal(0.0, 0.003));
        globalCommodityIndex_ = std::clamp(globalCommodityIndex_, 50.0, 250.0);

        // Pressão em cadeias de suprimento global
        double avgStability = 0.0; int cnt = 0;
        for (auto id : world_.getAllCountryIds()) {
            avgStability += world_.getCountry(id).internalStability;
            cnt++;
        }
        if (cnt > 0) avgStability /= cnt;
        globalSupplyChainPressure_ = std::clamp((0.5 - avgStability) * 0.5 +
            RandomEngine::instance().randNormal(0.0, 0.02), 0.0, 1.0);
    }
}

void EconomySystem::shutdown() {
    std::cout << "[Economy] Shutting down." << std::endl;
}

void EconomySystem::updateCountryEconomy(Country& country, double dt) {
    auto& tax = taxPolicies_[country.id];
    auto& budget = budgets_[country.id];
    auto& market = markets_[country.id];

    // ===== Cálculo de receita fiscal com efeito Laffer =====
    // taxEfficiency: mínimo ~40% (estado falido) até ~97% (país neto sem corrupção)
    // ruleOfLaw eleva a capacidade de arrecadar; corruption reduz via sonegação
    double taxEfficiency = std::clamp(
        0.50 + country.ruleOfLaw * 0.35 - country.corruption * 0.15,
        0.40, 0.97
    );

    // Efeito Laffer: alíquotas acima do ótimo reduzem a base tributária
    // Alíquota ótima estimada para IR: ~35%; cada ponto acima reduz base
    double weightedIncomeTax = tax.incomeTaxLow * 0.45 + tax.incomeTaxMid * 0.33 + tax.incomeTaxHigh * 0.22;
    double lafferFactorIncome = 1.0 - std::max(0.0, (weightedIncomeTax - 0.35) * 0.8);
    double lafferFactorCorp   = 1.0 - std::max(0.0, (tax.corporateTax   - 0.28) * 0.6);
    double lafferFactorSales  = 1.0 - std::max(0.0, (tax.salesTax       - 0.25) * 0.5);

    // --- Imposto de Renda + Contribuições Sociais ---
    double incomeRevenue = country.gdp.billions * 0.65 * weightedIncomeTax * taxEfficiency * lafferFactorIncome;

    // --- Imposto Corporativo ---
    double corpRevenue = country.gdp.billions * 0.12 * tax.corporateTax * taxEfficiency * lafferFactorCorp;

    // --- Impostos sobre Consumo ---
    double salesRevenue = country.gdp.billions * 0.68 * tax.salesTax * taxEfficiency * lafferFactorSales;

    // --- Tarifas de Importação ---
    double estimatedImports = country.gdp.billions * 0.22;
    double tariffRevenue = estimatedImports * tax.importTariff * taxEfficiency;

    // --- Taxa de Carbono ---
    double carbonRevenue = country.co2EmissionsMT * tax.carbonTax * 0.005 * taxEfficiency;

    // --- Imposto sobre Propriedade e Patrimônio ---
    double propRevenue = country.gdp.billions *
        (tax.propertyTax * 3.0 + tax.wealthTax * 2.0) * taxEfficiency;

    // --- Imposto sobre Ganhos de Capital ---
    double capGainsRevenue = country.gdp.billions * 0.08 * tax.capitalGainsTax * taxEfficiency;

    // --- Receitas não tributárias (royalties, dividendos de estatais, petróleo, etc.) ---
    double nonTaxRevenue = country.gdp.billions * 0.02 * country.ruleOfLaw;

    // --- Bônus por economia informal reduzida ---
    double& shadowRatio = shadowEconomies_[country.id].ratioToGdp;
    double formalBonus = country.gdp.billions * (0.15 - shadowRatio) * 0.05;

    double totalRevenue = incomeRevenue + corpRevenue + salesRevenue + tariffRevenue +
                          carbonRevenue + propRevenue + capGainsRevenue + nonTaxRevenue +
                          std::max(0.0, formalBonus);
    country.governmentRevenue = Money(totalRevenue);

    // ===== Cálculo de gastos =====
    double baseExpenditure = country.gdp.billions * budget.totalGdpRatio;
    double fiscalAdjustment = 0.0;
    if (country.debtToGDP > 0.9)       fiscalAdjustment -= 0.04;
    else if (country.debtToGDP > 0.7)  fiscalAdjustment -= 0.02;
    if (country.internalStability < 0.3) fiscalAdjustment += 0.01;

    // Crises bancárias forçam gasto emergencial
    if (bankingData_.count(country.id) && bankingData_[country.id].isBankingCrisis)
        fiscalAdjustment += 0.03;

    double totalExpenditure = baseExpenditure * (1.0 + fiscalAdjustment);
    country.governmentExpenditure = Money(totalExpenditure);

    // ===== Balanço fiscal =====
    double fiscalBalance = totalRevenue - totalExpenditure;
    country.governmentBudget = Money(fiscalBalance);

    // ===== Gestão do superávit / déficit =====
    double annualFlow = fiscalBalance * dt / 365.0;
    if (fiscalBalance > 0.0) {
        // Superávit: 70% para reservas, 30% para amortização de dívida
        country.foreignReserves += Money(annualFlow * 0.70);
        if (country.publicDebt.billions > 0.0)
            country.publicDebt -= Money(annualFlow * 0.30);
    } else {
        double needed = -annualFlow;
        if (country.foreignReserves.billions >= needed) {
            country.foreignReserves -= Money(needed);
        } else {
            double fromReserves = country.foreignReserves.billions;
            country.foreignReserves = Money(0.0);
            double leftover = needed - fromReserves;
            country.publicDebt += Money(leftover);
        }
    }
    if (country.foreignReserves.billions < 0.0) country.foreignReserves = Money(0.0);
    country.debtToGDP = country.gdp.billions > 0 ?
        country.publicDebt.billions / country.gdp.billions : 0.0;

    // ===== Crescimento do PIB =====
    double baseGrowth = 0.03;

    // Efeitos fiscais e tributários
    double taxEffect = -(weightedIncomeTax - 0.25) * 0.08
                       -(tax.corporateTax - 0.25) * 0.06;
    double educEffect   = budget.education * 0.06;
    double infraEffect  = budget.infrastructure * 0.05;
    double scienceEffect = budget.science * 0.04;
    double stabilityEffect  = (country.internalStability - 0.5) * 0.05;
    double corruptionEffect = -country.corruption * 0.04;
    double tradeEffect      = country.tradeBalance.billions > 0 ? 0.01 : -0.01;
    double interestEffect   = -(country.interestRate - 0.05) * 0.12;
    double debtEffect       = -(country.debtToGDP - 0.60) * 0.025;
    if (debtEffect > 0) debtEffect *= 0.4;

    // Efeito do preço do petróleo — exportadores ganham, importadores perdem
    double oilImpact = 0.0;
    if (country.oilProductionBarrelsDay > country.oilConsumptionBarrelsDay) {
        oilImpact = (globalOilPrice_ - 70.0) * 0.0003; // exportador beneficiado
    } else {
        oilImpact = -(globalOilPrice_ - 70.0) * 0.0002; // importador prejudicado
    }

    // Fator de produtividade total (TFP - Total Factor Productivity)
    double productivityFactor = computeProductivityFactor(country);
    productivityGrowth_[country.id] = productivityFactor;

    // Efeito do IED na economia real
    double fdiEffect = 0.0;
    if (fdiData_.count(country.id))
        fdiEffect = fdiData_[country.id].inflowsBillions / std::max(country.gdp.billions, 1.0) * 0.3;

    // Efeito da crise bancária
    double bankingEffect = 0.0;
    if (bankingData_.count(country.id)) {
        auto& bk = bankingData_[country.id];
        if (bk.isBankingCrisis) bankingEffect = -0.04;
        else bankingEffect = -(bk.nonPerformingLoanRatio - 0.05) * 0.1;
    }

    // Pressão de cadeia de suprimentos global
    double supplyEffect = -globalSupplyChainPressure_ * 0.01;

    double growthRate = baseGrowth + taxEffect + educEffect + infraEffect + scienceEffect +
                        stabilityEffect + corruptionEffect + tradeEffect +
                        interestEffect + debtEffect + oilImpact + productivityFactor * 0.5 +
                        fdiEffect + bankingEffect + supplyEffect;

    // Volatilidade estocástica
    growthRate += RandomEngine::instance().randNormal(0.0, 0.005) * config_.economicVolatility;

    country.gdpGrowthRate = growthRate;
    country.gdp.billions *= (1.0 + growthRate * dt / 365.0);
    if (country.population > 0)
        country.gdpPerCapita = country.gdp.billions * 1e9 / (country.population * 1e6);

    // ===== Inflação =====
    double baseInflation = 0.025;
    double monetaryInflation = std::clamp((0.04 - country.interestRate) * 0.5, -0.01, 0.03);
    double demandInflation   = (country.unemploymentRate < 0.045) ? 0.015 : -0.005;
    double debtInflation     = (country.debtToGDP > 0.85) ? 0.015 : 0.0;
    double oilInflation      = (globalOilPrice_ - 70.0) * 0.0003;
    double commodityInflation = (globalCommodityIndex_ - 100.0) * 0.0001;
    double supplyInflation   = globalSupplyChainPressure_ * 0.02;
    // Banco Central independente controla melhor a inflação
    double cbIndep = bankingData_.count(country.id) ? bankingData_[country.id].centralBankIndependence : 0.5;
    double inflationControl = -(cbIndep - 0.5) * 0.01;
    double inflNoise = RandomEngine::instance().randNormal(0.0, 0.002);

    country.inflation = std::clamp(
        baseInflation + monetaryInflation + demandInflation + debtInflation + oilInflation +
        commodityInflation + supplyInflation + inflationControl + inflNoise,
        -0.05, 0.60
    );

    // ===== Desemprego (Lei de Okun ampliada) =====
    double baseUnemployment = 0.05;
    double okunEffect = -(growthRate - 0.025) * 0.60;
    double techEffect = -0.008;
    double laborRegEffect = (weightedIncomeTax > 0.42) ? 0.012 : 0.0;
    // Crédito fácil estimula emprego; crédito caro desestimula
    double creditEffect = -(bankingData_.count(country.id) ?
        bankingData_[country.id].privateCreditGrowth - 0.05 : 0.0) * 0.1;
    double uneNoise = RandomEngine::instance().randNormal(0.0, 0.001);

    country.unemploymentRate = std::clamp(
        country.unemploymentRate + (okunEffect + techEffect + laborRegEffect + creditEffect + uneNoise) * dt / 365.0,
        0.01, 0.65
    );

    // ===== Mercado financeiro =====
    double businessConf = computeBusinessConfidence(country);
    businessConfidence_[country.id] = businessConf;
    double marketSentiment = growthRate * 5.0 + (country.internalStability - 0.5) * 2.0
                            + (businessConf - 0.5) * 1.5;
    market.stockMarketChange = marketSentiment + RandomEngine::instance().randNormal(0.0, 0.6);
    market.stockMarketIndex *= (1.0 + market.stockMarketChange * 0.01 * dt / 365.0);
    market.investorConfidence = std::clamp(
        market.investorConfidence + marketSentiment * 0.001 * dt, 0.0, 1.0);
    market.creditRating = std::clamp(
        1.0 - country.debtToGDP * 0.4 - country.inflation * 0.5 + country.ruleOfLaw * 0.2, 0.0, 1.0);
    double bondPremium = (1.0 - market.creditRating) * 0.08 + country.inflation;
    market.bondYield = std::clamp(country.interestRate + bondPremium, 0.001, 0.30);

    // ===== GINI (desigualdade) =====
    double taxProgressivity = (tax.incomeTaxHigh - tax.incomeTaxLow) * 0.12;
    double welfareEffect = budget.socialWelfare * 0.04;
    double educLong = budget.education * 0.02;
    country.giniCoefficient = std::clamp(
        country.giniCoefficient - (taxProgressivity + welfareEffect + educLong) * 0.001 * dt +
        (1.0 - budget.socialWelfare) * 0.0004 * dt + country.unemploymentRate * 0.001 * dt,
        0.20, 0.75
    );
}


void EconomySystem::updateGlobalTrade(double dt) {
    // Preços de commodities
    double oilDemand = 0.0;
    double oilSupply = 0.0;

    for (auto id : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(id);
        oilDemand += country.oilConsumptionBarrelsDay;
        oilSupply += country.oilProductionBarrelsDay;
    }

    // Preço do petróleo baseado em oferta/demanda
    if (oilSupply > 0) {
        double ratio = oilDemand / oilSupply;
        globalOilPrice_ *= (1.0 + (ratio - 1.0) * 0.01);
        globalOilPrice_ = std::clamp(globalOilPrice_, 20.0, 200.0);
    }

    // Preço do ouro (refúgio em crises)
    double globalStability = 0.0;
    int count = 0;
    for (auto id : world_.getAllCountryIds()) {
        globalStability += world_.getCountry(id).internalStability;
        count++;
    }
    if (count > 0) globalStability /= count;
    globalGoldPrice_ *= (1.0 + (0.5 - globalStability) * 0.005);
    globalGoldPrice_ = std::clamp(globalGoldPrice_, 800.0, 5000.0);
}

void EconomySystem::updateMarkets(double dt) {
    // Atualizar taxa de câmbio relativa
    for (auto id : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(id);
        auto& market = markets_[id];

        // Fatores que afetam câmbio
        double infDiff = country.inflation - 0.02;
        double interestDiff = country.interestRate - 0.05;
        double tradeDiff = country.tradeBalance.billions * 0.001;
        double confidenceDiff = (market.investorConfidence - 0.5) * 0.01;

        double exchangeChange = -infDiff * 0.01 + interestDiff * 0.005 +
                                tradeDiff + confidenceDiff;

        country.exchangeRate *= (1.0 + exchangeChange * dt / 365.0);
        country.exchangeRate = std::clamp(country.exchangeRate, 0.001, 100000.0);
    }
}

// ===== Ações do Jogador =====

void EconomySystem::setTaxPolicy(CountryID country, const TaxPolicy& policy) {
    taxPolicies_[country] = policy;
    auto& c = world_.getCountry(country);
    c.incomeTaxLow = policy.incomeTaxLow;
    c.incomeTaxMid = policy.incomeTaxMid;
    c.incomeTaxHigh = policy.incomeTaxHigh;
    c.corporateTax = policy.corporateTax;
    c.salesTax = policy.salesTax;
    c.importTariff = policy.importTariff;
    c.exportTariff = policy.exportTariff;
    c.carbonTax = policy.carbonTax;
}

void EconomySystem::setBudgetAllocation(CountryID country, const BudgetAllocation& budget) {
    double savedRatio = budgets_.count(country) ? budgets_[country].totalGdpRatio : 0.22;
    budgets_[country] = budget;
    budgets_[country].normalize();
    budgets_[country].totalGdpRatio = (budget.totalGdpRatio > 0) ? budget.totalGdpRatio : savedRatio;
    auto& c = world_.getCountry(country);
    c.budgetDefense = budgets_[country].defense;
    c.budgetEducation = budgets_[country].education;
    c.budgetHealthcare = budgets_[country].healthcare;
    c.budgetInfrastructure = budgets_[country].infrastructure;
    c.budgetSocialWelfare = budgets_[country].socialWelfare;
    c.budgetScience = budgets_[country].science;
    c.budgetAdministration = budgets_[country].administration;
    c.budgetDebtService = budgets_[country].debtService;
    c.budgetSecurity = budgets_[country].security;
    c.budgetAgriculture = budgets_[country].agriculture;
    c.budgetCulture = budgets_[country].culture;
    c.budgetTransport = budgets_[country].transport;
}

void EconomySystem::setInterestRate(CountryID country, double rate) {
    world_.getCountry(country).interestRate = std::clamp(rate, -0.01, 0.30);
}

void EconomySystem::subsidizeSector(CountryID country, SectorID sector, double amount) {
    auto& sec = world_.getSector(sector);
    sec.subsidies += amount;
    sec.growthRate += amount * 0.01; // Subsídio estimula crescimento
}

void EconomySystem::nationalizeSector(CountryID country, SectorID sector) {
    auto& sec = world_.getSector(sector);
    sec.stateOwned = true;
    sec.regulation = 1.0;
    // Impacto: mercado reage negativamente
    markets_[country].investorConfidence -= 0.05;
    markets_[country].stockMarketChange -= 2.0;
}

void EconomySystem::privatizeSector(CountryID country, SectorID sector) {
    auto& sec = world_.getSector(sector);
    sec.stateOwned = false;
    sec.regulation *= 0.7;
    // Impacto: mercado reage positivamente
    markets_[country].investorConfidence += 0.03;
    markets_[country].stockMarketChange += 1.5;
}

void EconomySystem::printMoney(CountryID country, double amountBillions) {
    auto& c = world_.getCountry(country);
    c.foreignReserves += Money(amountBillions);
    // Efeitos: inflação sobe, câmbio desvaloriza
    c.inflation += amountBillions * 0.001;
    c.exchangeRate *= (1.0 + amountBillions * 0.01);
}

void EconomySystem::issueDebt(CountryID country, double amountBillions) {
    auto& c = world_.getCountry(country);
    c.publicDebt += Money(amountBillions);
    c.foreignReserves += Money(amountBillions);
    c.debtToGDP = c.gdp.billions > 0 ? c.publicDebt.billions / c.gdp.billions : 0;
}

void EconomySystem::payDebt(CountryID country, double amountBillions) {
    auto& c = world_.getCountry(country);
    // Não pode pagar mais do que a dívida existente nem mais do que há nas reservas
    double actual = std::min({amountBillions, c.publicDebt.billions, c.foreignReserves.billions});
    actual = std::max(actual, 0.0);
    c.publicDebt -= Money(actual);
    c.foreignReserves -= Money(actual);
    if (c.foreignReserves.billions < 0.0) c.foreignReserves = Money(0.0);
    c.debtToGDP = c.gdp.billions > 0 ? c.publicDebt.billions / c.gdp.billions : 0;
}

void EconomySystem::regulateSector(CountryID country, SectorID sector, double level) {
    auto& sec = world_.getSector(sector);
    sec.regulation = std::clamp(level, 0.0, 1.0);
}

void EconomySystem::imposeTariff(CountryID country, CountryID target, double tariffRate) {
    // Afeta relações diplomáticas e comércio bilateral
    auto& rel = world_.getRelation(country, target);
    rel.relations -= tariffRate * 0.1;
    rel.tradeVolume *= (1.0 - tariffRate * 0.5);
}

void EconomySystem::liftTariff(CountryID country, CountryID target) {
    auto& rel = world_.getRelation(country, target);
    rel.relations += 0.02;
}

// ===== Consultas =====

TaxPolicy EconomySystem::getTaxPolicy(CountryID country) const {
    auto it = taxPolicies_.find(country);
    if (it != taxPolicies_.end()) return it->second;
    return {};
}

BudgetAllocation EconomySystem::getBudgetAllocation(CountryID country) const {
    auto it = budgets_.find(country);
    if (it != budgets_.end()) return it->second;
    return {};
}

MarketIndicators EconomySystem::getMarketIndicators(CountryID country) const {
    auto it = markets_.find(country);
    if (it != markets_.end()) return it->second;
    return {};
}

Money EconomySystem::calculateTaxRevenue(CountryID country) const {
    return world_.getCountry(country).governmentRevenue;
}

double EconomySystem::calculateDebtSustainability(CountryID country) const {
    auto& c = world_.getCountry(country);
    // Score 0-1 onde 1 é sustentável
    double debtScore = std::clamp(1.0 - c.debtToGDP, 0.0, 1.0);
    double growthScore = std::clamp(c.gdpGrowthRate * 10.0 + 0.5, 0.0, 1.0);
    return (debtScore * 0.6 + growthScore * 0.4);
}

std::vector<TradeData> EconomySystem::getTradePartners(CountryID country) const {
    auto it = trades_.find(country);
    if (it != trades_.end()) return it->second;
    return {};
}

double EconomySystem::calculateGDPGrowth(CountryID country) const {
    return world_.getCountry(country).gdpGrowthRate;
}

double EconomySystem::calculateInflation(CountryID country) const {
    return world_.getCountry(country).inflation;
}

double EconomySystem::calculateUnemployment(CountryID country) const {
    return world_.getCountry(country).unemploymentRate;
}

double EconomySystem::calculateFiscalBalance(CountryID country) const {
    // Retorna o balanço fiscal anual em bilhões (positivo = superávit, negativo = déficit)
    return world_.getCountry(country).governmentBudget.billions;
}

void EconomySystem::setSpendingRatio(CountryID country, double gdpPct) {
    budgets_[country].totalGdpRatio = std::clamp(gdpPct, 0.10, 0.55);
}

// ===== Novas Ações do Jogador =====

void EconomySystem::activateEconomicStimulus(CountryID country, double amountBillions) {
    auto& c = world_.getCountry(country);
    // Injeção direta de liquidez: aumento temporário de crescimento e emprego
    c.gdpGrowthRate += amountBillions / c.gdp.billions * 2.0;
    c.unemploymentRate = std::max(0.01, c.unemploymentRate - amountBillions / c.gdp.billions * 1.5);
    // Custo: aumento da dívida pública e pressão inflacionária
    c.publicDebt += Money(amountBillions);
    c.inflation += amountBillions / c.gdp.billions * 0.5;
    c.debtToGDP = c.gdp.billions > 0 ? c.publicDebt.billions / c.gdp.billions : 0;
    // Mercado reage positivamente no curto prazo
    if (markets_.count(country)) {
        markets_[country].stockMarketChange += amountBillions / c.gdp.billions * 5.0;
        markets_[country].investorConfidence = std::clamp(
            markets_[country].investorConfidence + 0.02, 0.0, 1.0);
    }
    std::cout << "[Economy] Stimulus of " << amountBillions << "B activated for country " << country << std::endl;
}

void EconomySystem::implementAusterityPackage(CountryID country, double cutRatio) {
    // cutRatio: 0.05 = corte de 5% nos gastos totais
    auto& c = world_.getCountry(country);
    double cut = std::clamp(cutRatio, 0.01, 0.30);
    if (budgets_.count(country)) {
        budgets_[country].totalGdpRatio *= (1.0 - cut);
    }
    // Impactos: crescimento cai, desemprego sobe, mas dívida melhora
    c.gdpGrowthRate -= cut * 0.5;
    c.unemploymentRate += cut * 0.3;
    c.internalStability -= cut * 0.2;
    // Mercado financeiro reage positivamente (credibilidade fiscal)
    if (markets_.count(country)) {
        markets_[country].creditRating = std::clamp(markets_[country].creditRating + cut * 0.3, 0.0, 1.0);
        markets_[country].bondYield = std::max(0.001, markets_[country].bondYield - cut * 0.02);
    }
    std::cout << "[Economy] Austerity package (" << cut * 100 << "% cuts) for country " << country << std::endl;
}

void EconomySystem::establishSovereignWealthFund(CountryID country, double initialBillions, double annualContribRate) {
    auto& fund = sovereignFunds_[country];
    fund.isActive = true;
    fund.fundBillions += initialBillions;
    fund.annualContribution = std::clamp(annualContribRate, 0.0, 0.30);
    // Deducted from reserves
    auto& c = world_.getCountry(country);
    double actual = std::min(initialBillions, c.foreignReserves.billions);
    c.foreignReserves -= Money(actual);
    fund.fundBillions = actual;
    std::cout << "[Economy] Sovereign Wealth Fund established: " << actual << "B for country " << country << std::endl;
}

void EconomySystem::setCentralBankIndependence(CountryID country, double level) {
    if (bankingData_.count(country))
        bankingData_[country].centralBankIndependence = std::clamp(level, 0.0, 1.0);
}

void EconomySystem::bankBailout(CountryID country, double amountBillions) {
    if (!bankingData_.count(country)) return;
    auto& bk = bankingData_[country];
    auto& c = world_.getCountry(country);
    // Bailout: reduz NPL, capitaliza bancos, aumenta dívida pública
    bk.nonPerformingLoanRatio = std::max(0.01, bk.nonPerformingLoanRatio - amountBillions / c.gdp.billions * 0.3);
    bk.capitalAdequacyRatio   = std::min(0.25,  bk.capitalAdequacyRatio  + amountBillions / c.gdp.billions * 0.2);
    bk.bankingCrisisRisk      = std::max(0.0,   bk.bankingCrisisRisk     - 0.3);
    if (bk.bankingCrisisRisk < 0.2) bk.isBankingCrisis = false;
    c.publicDebt += Money(amountBillions);
    c.debtToGDP = c.gdp.billions > 0 ? c.publicDebt.billions / c.gdp.billions : 0;
    std::cout << "[Economy] Bank bailout: " << amountBillions << "B for country " << country << std::endl;
}

void EconomySystem::formalizeShadowEconomy(CountryID country, double policyStrength) {
    // policyStrength: 0-1, esforço de formalização
    if (!shadowEconomies_.count(country)) return;
    auto& shadow = shadowEconomies_[country];
    double effect = std::clamp(policyStrength, 0.0, 1.0) * 0.02;
    shadow.ratioToGdp = std::max(0.03, shadow.ratioToGdp - effect);
    shadow.formalizationTrend = effect;
    shadow.informelEmploymentRatio = std::max(0.05, shadow.informelEmploymentRatio - effect * 1.5);
}

// ===== Consultas Novas =====

EconomicCyclePhase EconomySystem::getEconomicCyclePhase(CountryID country) const {
    auto it = economicCycles_.find(country);
    if (it != economicCycles_.end()) return it->second;
    return EconomicCyclePhase::EXPANSION;
}

std::string EconomySystem::getEconomicCycleName(CountryID country) const {
    switch (getEconomicCyclePhase(country)) {
        case EconomicCyclePhase::EXPANSION:   return "Expansão";
        case EconomicCyclePhase::PEAK:        return "Pico / Sobreaquecimento";
        case EconomicCyclePhase::CONTRACTION: return "Desaceleração";
        case EconomicCyclePhase::RECESSION:   return "Recessão";
        case EconomicCyclePhase::TROUGH:      return "Fundo do Ciclo";
        case EconomicCyclePhase::RECOVERY:    return "Recuperação";
        default: return "Desconhecido";
    }
}

FDIIndicators EconomySystem::getFDIIndicators(CountryID country) const {
    auto it = fdiData_.find(country);
    if (it != fdiData_.end()) return it->second;
    return {};
}

BankingIndicators EconomySystem::getBankingIndicators(CountryID country) const {
    auto it = bankingData_.find(country);
    if (it != bankingData_.end()) return it->second;
    return {};
}

SectoralBreakdown EconomySystem::getSectoralBreakdown(CountryID country) const {
    auto it = sectors_.find(country);
    if (it != sectors_.end()) return it->second;
    return {};
}

ShadowEconomy EconomySystem::getShadowEconomy(CountryID country) const {
    auto it = shadowEconomies_.find(country);
    if (it != shadowEconomies_.end()) return it->second;
    return {};
}

IncomeDistribution EconomySystem::getIncomeDistribution(CountryID country) const {
    auto it = incomeDistributions_.find(country);
    if (it != incomeDistributions_.end()) return it->second;
    return {};
}

SovereignWealthData EconomySystem::getSovereignWealthData(CountryID country) const {
    auto it = sovereignFunds_.find(country);
    if (it != sovereignFunds_.end()) return it->second;
    return {};
}

double EconomySystem::getPovertyRate(CountryID country) const {
    auto it = incomeDistributions_.find(country);
    if (it != incomeDistributions_.end()) return it->second.povertyRate;
    return 0.20;
}

double EconomySystem::getProductivityGrowth(CountryID country) const {
    auto it = productivityGrowth_.find(country);
    if (it != productivityGrowth_.end()) return it->second;
    return 0.01;
}

double EconomySystem::getBusinessConfidenceIndex(CountryID country) const {
    auto it = businessConfidence_.find(country);
    if (it != businessConfidence_.end()) return it->second;
    return 0.5;
}

double EconomySystem::getLafferOptimalTax(CountryID country) const {
    // Estima a alíquota ótima com base na qualidade institucional
    // Países com alta qualidade institucional suportam alíquotas mais altas
    auto& c = world_.getCountry(country);
    return std::clamp(0.30 + c.ruleOfLaw * 0.10 - c.corruption * 0.05, 0.20, 0.50);
}

// ===== Novos Subsistemas Privados =====

void EconomySystem::updateBankingSector(Country& country, double dt) {
    if (!bankingData_.count(country.id)) return;
    auto& bk = bankingData_[country.id];
    auto& tax = taxPolicies_[country.id];

    // NPL aumenta com recessão e desemprego
    double nplChange = 0.0;
    if (country.gdpGrowthRate < 0.0) nplChange += -country.gdpGrowthRate * 0.05;
    nplChange += country.unemploymentRate * 0.005;
    nplChange -= country.ruleOfLaw * 0.01; // boa regulação reduz inadimplência
    bk.nonPerformingLoanRatio = std::clamp(
        bk.nonPerformingLoanRatio + nplChange * dt / 365.0, 0.005, 0.40);

    // Índice de Basileia (CAR)
    bk.capitalAdequacyRatio = std::clamp(
        bk.capitalAdequacyRatio - bk.nonPerformingLoanRatio * 0.05 * dt / 365.0 +
        country.ruleOfLaw * 0.005 * dt / 365.0, 0.04, 0.30);

    // Crescimento do crédito privado
    double creditTarget = std::clamp(0.50 + country.ruleOfLaw * 0.60, 0.20, 1.80);
    double creditGap = creditTarget - bk.creditToGdp;
    bk.privateCreditGrowth = std::clamp(
        country.interestRate < 0.06 ? 0.08 - country.interestRate : 0.03,
        -0.05, 0.20);
    bk.creditToGdp = std::clamp(
        bk.creditToGdp + creditGap * 0.05 * dt / 365.0 * bk.privateCreditGrowth, 0.10, 2.50);

    // Taxa interbancária
    bk.interbankRate = std::clamp(country.interestRate + bk.nonPerformingLoanRatio * 0.5, 0.001, 0.25);

    // Risco de crise sistêmica (critérios de alerta precoce)
    double crisisRisk = 0.0;
    if (bk.nonPerformingLoanRatio > 0.10) crisisRisk += (bk.nonPerformingLoanRatio - 0.10) * 2.0;
    if (bk.capitalAdequacyRatio < 0.08)  crisisRisk += (0.08 - bk.capitalAdequacyRatio) * 3.0;
    if (bk.creditToGdp > 1.20)            crisisRisk += (bk.creditToGdp - 1.20) * 0.5;
    if (country.debtToGDP > 1.0)          crisisRisk += (country.debtToGDP - 1.0) * 0.3;
    bk.bankingCrisisRisk = std::clamp(crisisRisk + RandomEngine::instance().randNormal(0.0, 0.01), 0.0, 1.0);

    // Declarar crise se risco for alto
    if (!bk.isBankingCrisis && bk.bankingCrisisRisk > 0.75) {
        bk.isBankingCrisis = true;
        std::cout << "[Economy] *** BANKING CRISIS triggered in country " << country.id << " ***" << std::endl;
        // Choque imediato
        country.gdpGrowthRate -= 0.05;
        country.unemploymentRate = std::min(0.65, country.unemploymentRate + 0.04);
        if (markets_.count(country.id))
            markets_[country.id].stockMarketChange -= 15.0;
    } else if (bk.isBankingCrisis && bk.bankingCrisisRisk < 0.30) {
        bk.isBankingCrisis = false;
    }
}

void EconomySystem::updateFDIFlows(Country& country, double dt) {
    if (!fdiData_.count(country.id)) return;
    auto& fdi = fdiData_[country.id];
    auto& market = markets_[country.id];

    // IED atraído por: ambiente de negócios, estabilidade, crescimento, custo fiscal
    double growthAttraction   = country.gdpGrowthRate * 2.0;
    double stabilityBonus     = country.internalStability * 0.5;
    double corruptionPenalty  = -country.corruption * 0.8;
    double taxAttraction      = -(taxPolicies_.count(country.id) ?
        taxPolicies_[country.id].corporateTax - 0.25 : 0.0) * 1.0;
    double infrastructureBonus = country.budgetInfrastructure * 0.5;
    double edbBase = std::clamp(
        0.20 + growthAttraction + stabilityBonus + corruptionPenalty + taxAttraction + infrastructureBonus,
        0.05, 0.95);
    fdi.easeOfDoingBusiness += (edbBase - fdi.easeOfDoingBusiness) * 0.05 * dt / 365.0;

    // Fluxo anual de IED: proporcional ao tamanho da economia e atratividade
    double targetInflow = country.gdp.billions * fdi.easeOfDoingBusiness * 0.05
                         - country.gdp.billions * (1.0 - country.internalStability) * 0.02;
    fdi.inflowsBillions += (targetInflow - fdi.inflowsBillions) * 0.20 * dt / 365.0;
    fdi.inflowsBillions = std::max(0.0, fdi.inflowsBillions);

    // Fuga de capitais quando há instabilidade
    fdi.outflowsBillions = country.gdp.billions *
        std::max(0.0, (1.0 - country.internalStability) * 0.03 + country.corruption * 0.01);
    if (bankingData_.count(country.id) && bankingData_[country.id].isBankingCrisis)
        fdi.outflowsBillions *= 3.0;

    // Estoque acumula entradas e perde saídas + depreciação de 5%/ano
    fdi.stockBillions += (fdi.inflowsBillions - fdi.outflowsBillions - fdi.stockBillions * 0.05) * dt / 365.0;
    fdi.stockBillions = std::max(0.0, fdi.stockBillions);
    fdi.fdiToGdpRatio = country.gdp.billions > 0 ? fdi.stockBillions / country.gdp.billions : 0;
    fdi.reinvestedEarnings = fdi.stockBillions * 0.08; // retorno médio de 8% reinvestido

    // Prêmio de risco político
    fdi.politicalRiskPremium = std::clamp(
        (1.0 - country.internalStability) * 400.0 + country.corruption * 200.0 - country.ruleOfLaw * 100.0,
        0.0, 800.0);

    // Reservas sobem com entrada líquida de IED
    double netFDI = (fdi.inflowsBillions - fdi.outflowsBillions) * dt / 365.0;
    country.foreignReserves += Money(netFDI * 0.3);
}

void EconomySystem::updateSectoralBreakdown(Country& country, double dt) {
    if (!sectors_.count(country.id)) return;
    auto& s = sectors_[country.id];
    auto& budget = budgets_[country.id];

    // Dinâmica setorial: setores crescem em resposta a gastos, política e mercado global
    double agriGrowth  = s.agricultureGrowth + budget.agriculture * 0.05
                        + (globalCommodityIndex_ - 100.0) * 0.0002;
    double industGrowth = s.industryGrowth + budget.infrastructure * 0.04
                        - taxPolicies_[country.id].corporateTax * 0.05;
    double servGrowth  = s.servicesGrowth + budget.healthcare * 0.02 + budget.education * 0.02;
    double techGrowth  = s.technologyGrowth + budget.science * 0.08 + budget.education * 0.04;
    double enGrowth    = s.energyGrowth + (globalOilPrice_ > 80.0 ? 0.01 : -0.005);
    double finGrowth   = s.financeGrowth + (bankingData_.count(country.id) ?
        bankingData_[country.id].privateCreditGrowth * 0.1 : 0.02);

    // Ajustar pesos sectoriais com base no crescimento diferencial
    s.agriculture  *= (1.0 + agriGrowth  * dt / 365.0);
    s.industry     *= (1.0 + industGrowth * dt / 365.0);
    s.services     *= (1.0 + servGrowth  * dt / 365.0);
    s.technology   *= (1.0 + techGrowth  * dt / 365.0);
    s.energy       *= (1.0 + enGrowth    * dt / 365.0);
    s.finance      *= (1.0 + finGrowth   * dt / 365.0);

    // Normalizar para somar 1.0
    double total = s.agriculture + s.industry + s.services + s.technology + s.energy + s.finance;
    if (total > 0) {
        s.agriculture /= total; s.industry /= total; s.services /= total;
        s.technology  /= total; s.energy   /= total; s.finance  /= total;
    }

    // Atualizar taxas de crescimento dos setores
    s.agricultureGrowth = agriGrowth;
    s.industryGrowth    = industGrowth;
    s.servicesGrowth    = servGrowth;
    s.technologyGrowth  = techGrowth;
    s.energyGrowth      = enGrowth;
    s.financeGrowth     = finGrowth;
}

void EconomySystem::updateShadowEconomy(Country& country, double dt) {
    if (!shadowEconomies_.count(country.id)) return;
    auto& shadow = shadowEconomies_[country.id];
    auto& tax = taxPolicies_[country.id];

    // Alta tributação → mais informal; mais rule of law → menos informal
    double weightedTax = tax.incomeTaxLow * 0.45 + tax.incomeTaxMid * 0.33 + tax.incomeTaxHigh * 0.22;
    double informalPush = (weightedTax - 0.25) * 0.05;   // impostos altos empurram para informal
    double formalPull   = country.ruleOfLaw * 0.03        // estado de direito facilita formalização
                         + country.budgetSocialWelfare * 0.02; // benefícios atraem formalização
    double shadowDelta  = (informalPush - formalPull) * dt / 365.0
                         + shadow.formalizationTrend * (-dt / 365.0);

    shadow.ratioToGdp = std::clamp(shadow.ratioToGdp + shadowDelta, 0.03, 0.70);
    shadow.informelEmploymentRatio = std::clamp(shadow.ratioToGdp * 1.8, 0.05, 0.85);
    shadow.taxGapBillions = country.gdp.billions * shadow.ratioToGdp *
        (weightedTax + tax.salesTax) * 0.4;
    shadow.formalizationTrend *= 0.95; // tendência de formalização decai se não renovada
}

void EconomySystem::updateEconomicCycle(Country& country, double dt) {
    auto& phase = economicCycles_[country.id];
    auto& rqtr  = recessionQuarters_[country.id];
    double g    = country.gdpGrowthRate;

    // Contagem de trimestres em recessão
    if (g < 0.0) rqtr++;
    else         rqtr = std::max(0, rqtr - 1);

    // Transições de fase
    switch (phase) {
        case EconomicCyclePhase::EXPANSION:
            if (country.inflation > 0.06 && country.unemploymentRate < 0.04)
                phase = EconomicCyclePhase::PEAK;
            else if (g < 0.01)
                phase = EconomicCyclePhase::CONTRACTION;
            break;
        case EconomicCyclePhase::PEAK:
            if (g < 0.015)   phase = EconomicCyclePhase::CONTRACTION;
            break;
        case EconomicCyclePhase::CONTRACTION:
            if (g < 0.0)     phase = EconomicCyclePhase::RECESSION;
            else if (g > 0.03) phase = EconomicCyclePhase::EXPANSION;
            break;
        case EconomicCyclePhase::RECESSION:
            if (rqtr >= 2 && g < 0.0) phase = EconomicCyclePhase::TROUGH;
            else if (g > 0.01)         phase = EconomicCyclePhase::RECOVERY;
            break;
        case EconomicCyclePhase::TROUGH:
            if (g > 0.0)     phase = EconomicCyclePhase::RECOVERY;
            break;
        case EconomicCyclePhase::RECOVERY:
            if (g > 0.03)    phase = EconomicCyclePhase::EXPANSION;
            else if (g < 0.0) phase = EconomicCyclePhase::RECESSION;
            break;
    }
}

void EconomySystem::updateIncomeDistribution(Country& country, double dt) {
    if (!incomeDistributions_.count(country.id)) return;
    auto& income = incomeDistributions_[country.id];
    auto& budget = budgets_[country.id];

    // Pobreza: melhora com crescimento, welfare, educação; piora com desemprego
    double povertyDelta = -country.gdpGrowthRate * 0.10
                         -budget.socialWelfare * 0.03
                         -budget.education * 0.02
                         +country.unemploymentRate * 0.08
                         +country.inflation * 0.05;
    income.povertyRate = std::clamp(income.povertyRate + povertyDelta * dt / 365.0, 0.01, 0.90);
    income.extremePovertyRate = income.povertyRate * std::clamp(
        0.30 - budget.socialWelfare * 0.20, 0.05, 0.50);

    // Mobilidade social melhora com educação e rule of law
    income.socialMobilityIndex = std::clamp(
        income.socialMobilityIndex +
        (budget.education * 0.04 + country.ruleOfLaw * 0.02 - country.corruption * 0.02) * dt / 365.0,
        0.05, 0.95);

    // Concentração de renda reflete Gini
    income.incomeShareTop10 = std::clamp(
        0.20 + country.giniCoefficient * 0.52, 0.20, 0.70);
    income.incomeShareBottom50 = std::clamp(
        0.40 - country.giniCoefficient * 0.40, 0.05, 0.40);

    // Classe média
    income.middleClassShare = std::clamp(
        1.0 - income.povertyRate - income.incomeShareTop10 * 0.15, 0.10, 0.80);
}

void EconomySystem::updateSovereignWealthFund(Country& country, double dt) {
    if (!sovereignFunds_.count(country.id)) return;
    auto& fund = sovereignFunds_[country.id];
    if (!fund.isActive) return;

    // Rendimento anual do fundo: ~5% em média (mix renda fixa + variável)
    double annualReturn = fund.fundBillions * 0.05;
    // Contribuição anual da receita de royalties
    double royaltyContrib = country.gdp.billions * fund.annualContribution * dt / 365.0;
    fund.fundBillions += (annualReturn + royaltyContrib) * dt / 365.0;
    fund.fundBillions = std::max(0.0, fund.fundBillions);

    // Em crise, o fundo pode ser usado para estabilização
    if (country.debtToGDP > 1.20 && country.foreignReserves.billions < 10.0) {
        double withdrawal = std::min(fund.fundBillions * 0.10, 50.0);
        fund.fundBillions -= withdrawal;
        country.foreignReserves += Money(withdrawal);
        std::cout << "[Economy] SWF withdrawal of " << withdrawal << "B for fiscal stabilization." << std::endl;
    }
}

// ===== Produtividade e Confiança Empresarial =====

double EconomySystem::computeProductivityFactor(const Country& country) const {
    // TFP: educação, investimento em P&D, infraestrutura, abertura comercial, qualidade institucional
    auto& b = budgets_.at(country.id);
    double tfp = b.education * 0.10
               + b.science * 0.15
               + b.infrastructure * 0.08
               + country.ruleOfLaw * 0.04
               - country.corruption * 0.03
               + (sectors_.count(country.id) ? sectors_.at(country.id).technology * 0.05 : 0.0)
               + (fdiData_.count(country.id)  ? fdiData_.at(country.id).fdiToGdpRatio * 0.05 : 0.0);
    return std::clamp(tfp, 0.0, 0.05);
}

double EconomySystem::computeBusinessConfidence(const Country& country) const {
    double conf = 0.50;
    conf += country.internalStability * 0.20;
    conf += country.ruleOfLaw * 0.15;
    conf -= country.corruption * 0.15;
    conf += country.gdpGrowthRate * 2.0;
    conf -= country.inflation * 0.5;
    if (bankingData_.count(country.id)) {
        conf -= bankingData_.at(country.id).bankingCrisisRisk * 0.20;
        conf += (bankingData_.at(country.id).centralBankIndependence - 0.5) * 0.05;
    }
    return std::clamp(conf, 0.05, 0.98);
}

double EconomySystem::applyLafferCurveCorrection(double nominalTaxRevenue, double weightedTaxRate,
                                                   double optimalRate, double taxEfficiency) {
    // Se alíquota acima do ótimo, aplica penalidade progressiva
    if (weightedTaxRate > optimalRate) {
        double excess = weightedTaxRate - optimalRate;
        double correction = 1.0 - std::min(0.50, excess * 1.5);
        return nominalTaxRevenue * correction;
    }
    return nominalTaxRevenue;
}

} // namespace GPS
