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
    }
}

void EconomySystem::update(double deltaTime, const SimDate& currentDate) {
    for (auto id : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(id);
        updateCountryEconomy(country, deltaTime);
    }

    // Comércio global atualizado diariamente
    if (currentDate.hour == 0) {
        updateGlobalTrade(deltaTime);
        updateMarkets(deltaTime);
    }
}

void EconomySystem::shutdown() {
    std::cout << "[Economy] Shutting down." << std::endl;
}

void EconomySystem::updateCountryEconomy(Country& country, double dt) {
    auto& tax = taxPolicies_[country.id];
    auto& budget = budgets_[country.id];
    auto& market = markets_[country.id];

    // ===== Cálculo de receita fiscal =====
    // taxEfficiency: mínimo ~40% (estado falido) até ~97% (país neto sem corrupção)
    // ruleOfLaw eleva a capacidade de arrecadar; corruption reduz via sonegação
    double taxEfficiency = std::clamp(
        0.50 + country.ruleOfLaw * 0.35 - country.corruption * 0.15,
        0.40, 0.97
    );

    // --- Imposto de Renda + Contribuições Sociais ---
    // Base = 65% do PIB (renda trabalho + contribuições patronais sobre salários)
    // Brackets modelam TODOS encargos sobre renda (IRPF + INSS + similares)
    double incomeRevenue = country.gdp.billions * 0.65 *
        (tax.incomeTaxLow * 0.45 + tax.incomeTaxMid * 0.33 + tax.incomeTaxHigh * 0.22) *
        taxEfficiency;

    // --- Imposto Corporativo ---
    // Lucro corporativo ≈ 12% do PIB em média
    double corpRevenue = country.gdp.billions * 0.12 * tax.corporateTax * taxEfficiency;

    // --- Impostos sobre Consumo (todos os tributos indiretos somados) ---
    // Base = 68% do PIB (consumo privado); salesTax representa TODOS impostos
    // indiretos (IVA, ICMS, PIS/COFINS, ISS, IPI etc. conforme o país)
    double salesRevenue = country.gdp.billions * 0.68 * tax.salesTax * taxEfficiency;

    // --- Tarifas de Importação ---
    // Importações estimadas em ~22% do PIB
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

    double totalRevenue = incomeRevenue + corpRevenue + salesRevenue + tariffRevenue +
                          carbonRevenue + propRevenue + capGainsRevenue + nonTaxRevenue;
    country.governmentRevenue = Money(totalRevenue);

    // ===== Cálculo de gastos (% fixa do PIB — controlada pelo jogador) =====
    // Os gastos NÃO seguem a receita: são fixados em totalGdpRatio * PIB.
    // Isso permite que variações de imposto criem superávit ou déficit real.
    double baseExpenditure = country.gdp.billions * budget.totalGdpRatio;

    // Ajuste conjuntural automático: dívida muito alta aplica austeridade
    double fiscalAdjustment = 0.0;
    if (country.debtToGDP > 0.9)       fiscalAdjustment -= 0.04;
    else if (country.debtToGDP > 0.7)  fiscalAdjustment -= 0.02;
    if (country.internalStability < 0.3) fiscalAdjustment += 0.01; // gasto emergencial

    double totalExpenditure = baseExpenditure * (1.0 + fiscalAdjustment);
    country.governmentExpenditure = Money(totalExpenditure);

    // ===== Balanço fiscal (superávit = positivo, déficit = negativo) =====
    double fiscalBalance = totalRevenue - totalExpenditure;
    country.governmentBudget = Money(fiscalBalance);

    // ===== Gestão do superávit e do déficit =====
    double annualFlow = fiscalBalance * dt / 365.0;

    if (fiscalBalance > 0.0) {
        // SUPERÁVIT: vai integralmente para as reservas internacionais
        country.foreignReserves += Money(annualFlow);
        // Bônus: reservas elevadas melhoram rating de crédito
    } else {
        // DÉFICIT: drena reservas primeiro; só emite dívida após esgotá-las
        double needed = -annualFlow; // valor positivo que precisa ser coberto
        if (country.foreignReserves.billions >= needed) {
            country.foreignReserves -= Money(needed);
        } else {
            // Reservas insuficientes: zera e emite o restante como dívida
            double fromReserves = country.foreignReserves.billions;
            country.foreignReserves = Money(0.0);
            double leftover = needed - fromReserves;
            country.publicDebt += Money(leftover);
        }
    }

    // Reservas não podem ser negativas
    if (country.foreignReserves.billions < 0.0)
        country.foreignReserves = Money(0.0);
    country.debtToGDP = country.gdp.billions > 0 ?
        country.publicDebt.billions / country.gdp.billions : 0.0;

    // ===== Crescimento do PIB =====
    double baseGrowth = 0.03; // 3% base

    // Fatores que afetam crescimento
    double taxEffect = -(tax.corporateTax - 0.25) * 0.1; // Impostos altos reduzem crescimento
    double infraEffect = 0.0; // Vem do InfrastructureSystem
    double educEffect = budget.education * 0.05;
    double stabilityEffect = (country.internalStability - 0.5) * 0.04;
    double corruptionEffect = -country.corruption * 0.03;
    double tradeEffect = country.tradeBalance.billions > 0 ? 0.01 : -0.01;
    double interestEffect = -(country.interestRate - 0.05) * 0.1;
    double debtEffect = -(country.debtToGDP - 0.6) * 0.02;
    if (debtEffect > 0) debtEffect *= 0.5; // Menos benefício por dívida baixa

    double growthRate = baseGrowth + taxEffect + infraEffect + educEffect +
                        stabilityEffect + corruptionEffect + tradeEffect +
                        interestEffect + debtEffect;

    // Volatilidade
    growthRate += RandomEngine::instance().randNormal(0.0, 0.005) * config_.economicVolatility;

    // Aplicar crescimento (diário)
    country.gdpGrowthRate = growthRate;
    country.gdp.billions *= (1.0 + growthRate * dt / 365.0);
    if (country.population > 0) {
        country.gdpPerCapita = country.gdp.billions * 1e9 / (country.population * 1e6);
    }

    // ===== Inflação =====
    double baseInflation = 0.02;
    double monetaryInflation = (country.interestRate < 0.03) ? 0.01 : -0.005;
    double demandInflation = (country.unemploymentRate < 0.04) ? 0.01 : -0.005;
    double debtInflation = (country.debtToGDP > 0.8) ? 0.02 : 0.0;
    double inflNoise = RandomEngine::instance().randNormal(0.0, 0.002);

    country.inflation = std::clamp(
        baseInflation + monetaryInflation + demandInflation + debtInflation + inflNoise,
        -0.05, 0.50
    );

    // ===== Desemprego =====
    double baseUnemployment = 0.05;
    double growthEffect2 = -(growthRate - 0.02) * 0.5; // Lei de Okun simplificada
    double techEffect = -0.01; // Tecnologia cria empregos no longo prazo
    double laborRegEffect = (tax.incomeTaxHigh > 0.4) ? 0.01 : 0.0;
    double uneNoise = RandomEngine::instance().randNormal(0.0, 0.001);

    country.unemploymentRate = std::clamp(
        country.unemploymentRate + (growthEffect2 + techEffect + laborRegEffect + uneNoise) * dt / 365.0,
        0.01, 0.60
    );

    // ===== Mercado financeiro =====
    double marketSentiment = growthRate * 5.0 + (country.internalStability - 0.5) * 2.0;
    market.stockMarketChange = marketSentiment + RandomEngine::instance().randNormal(0.0, 0.5);
    market.stockMarketIndex *= (1.0 + market.stockMarketChange * 0.01 * dt / 365.0);
    market.investorConfidence = std::clamp(
        market.investorConfidence + marketSentiment * 0.001 * dt,
        0.0, 1.0
    );
    market.creditRating = std::clamp(1.0 - country.debtToGDP * 0.5, 0.0, 1.0);

    // ===== Gini (desigualdade) =====
    double taxProgressivity = (tax.incomeTaxHigh - tax.incomeTaxLow) * 0.1;
    country.giniCoefficient = std::clamp(
        country.giniCoefficient - taxProgressivity * 0.001 * dt +
        (1.0 - budget.socialWelfare) * 0.0005 * dt,
        0.2, 0.7
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
    // gdpPct: fração do PIB destinada a gastos públicos (ex: 0.22 = 22%)
    budgets_[country].totalGdpRatio = std::clamp(gdpPct, 0.10, 0.55);
}

} // namespace GPS
