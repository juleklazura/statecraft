/**
 * GPS - Geopolitical Simulator
 * EconomySystem.h - Sistema econômico nacional e global
 * 
 * Simula macroeconomia realista: PIB, inflação, desemprego,
 * setores produtivos, comércio internacional, orçamento público,
 * dívida, mercado financeiro e política fiscal.
 */
#pragma once

#include "core/SimulationEngine.h"
#include "world/WorldState.h"

namespace GPS {

struct TaxPolicy {
    double incomeTaxLow = 0.10;
    double incomeTaxMid = 0.20;
    double incomeTaxHigh = 0.35;
    double corporateTax = 0.25;
    double salesTax = 0.10;
    double importTariff = 0.05;
    double exportTariff = 0.01;
    double carbonTax = 0.00;
    double capitalGainsTax = 0.15;
    double propertyTax = 0.01;
    double wealthTax = 0.00;
};

struct BudgetAllocation {
    double defense = 0.10;
    double education = 0.14;
    double healthcare = 0.12;
    double infrastructure = 0.07;
    double socialWelfare = 0.18;
    double science = 0.05;
    double administration = 0.06;
    double debtService = 0.10;
    double environment = 0.03;
    double security = 0.06;      // Segurança Pública / Criminal / Imigração
    double agriculture = 0.04;   // Agricultura / Outros rurais
    double culture = 0.02;       // Mídia / Cultura / Esporte
    double transport = 0.03;     // Transporte Público (adicional a infraestr.)

    // Gastos do governo como fração do PIB (ex: 0.22 = 22% do PIB).
    // É fixo/controlado pelo jogador. A RECEITA varia com as alíquotas,
    // então aumentar impostos → superávit; reduzir impostos → déficit.
    double totalGdpRatio = 0.22;

    double total() const {
        return defense + education + healthcare + infrastructure +
               socialWelfare + science + administration + debtService +
               environment + security + agriculture + culture + transport;
    }

    void normalize() {
        double t = total();
        if (t > 0) {
            defense /= t;       education /= t;    healthcare /= t;
            infrastructure /= t; socialWelfare /= t; science /= t;
            administration /= t; debtService /= t; environment /= t;
            security /= t;      agriculture /= t;  culture /= t;
            transport /= t;
        }
    }
};

struct MarketIndicators {
    double stockMarketIndex = 1000.0;
    double stockMarketChange = 0.0;
    double bondYield = 0.03;
    double currencyStrength = 1.0;
    double investorConfidence = 0.5;
    double creditRating = 0.7;  // 0-1, onde 1 = AAA
    double foreignInvestment = 0.0;
    double capitalFlight = 0.0;
};

struct TradeData {
    CountryID partner;
    double exports = 0.0;   // Bilhões
    double imports = 0.0;
    std::vector<SectorID> mainExportSectors;
    std::vector<SectorID> mainImportSectors;
};

class EconomySystem : public ISystem {
public:
    explicit EconomySystem(WorldState& world, const SimulationConfig& config);

    void init() override;
    void update(double deltaTime, const SimDate& currentDate) override;
    void shutdown() override;
    const char* getName() const override { return "EconomySystem"; }
    int getPriority() const override { return 10; }

    // Ações do jogador
    void setTaxPolicy(CountryID country, const TaxPolicy& policy);
    void setBudgetAllocation(CountryID country, const BudgetAllocation& budget);
    void setInterestRate(CountryID country, double rate);
    void subsidizeSector(CountryID country, SectorID sector, double amount);
    void regulateSector(CountryID country, SectorID sector, double level);
    void nationalizeSector(CountryID country, SectorID sector);
    void privatizeSector(CountryID country, SectorID sector);
    void imposeTariff(CountryID country, CountryID target, double tariffRate);
    void liftTariff(CountryID country, CountryID target);
    void printMoney(CountryID country, double amountBillions);
    void issueDebt(CountryID country, double amountBillions);
    void payDebt(CountryID country, double amountBillions);

    // Consultas
    TaxPolicy getTaxPolicy(CountryID country) const;
    BudgetAllocation getBudgetAllocation(CountryID country) const;
    MarketIndicators getMarketIndicators(CountryID country) const;
    std::vector<TradeData> getTradePartners(CountryID country) const;
    double calculateGDPGrowth(CountryID country) const;
    double calculateInflation(CountryID country) const;
    double calculateUnemployment(CountryID country) const;
    double calculateDebtSustainability(CountryID country) const;
    Money calculateTaxRevenue(CountryID country) const;
    double calculateFiscalBalance(CountryID country) const;  // positivo=superávit, negativo=déficit
    void setSpendingRatio(CountryID country, double gdpPct); // 0.15 a 0.55 — gastos como % do PIB

private:
    void updateCountryEconomy(Country& country, double dt);
    void updateGlobalTrade(double dt);
    void updateMarkets(double dt);
    void recalculateSectors(Country& country);
    void processMonetaryPolicy(Country& country);
    void processFiscalPolicy(Country& country);
    double calculateSectorGrowth(const Country& country, const EconomicSector& sector);

    WorldState& world_;
    const SimulationConfig& config_;

    std::unordered_map<CountryID, TaxPolicy> taxPolicies_;
    std::unordered_map<CountryID, BudgetAllocation> budgets_;
    std::unordered_map<CountryID, MarketIndicators> markets_;
    std::unordered_map<CountryID, std::vector<TradeData>> trades_;

    double globalOilPrice_ = 70.0;
    double globalGoldPrice_ = 1800.0;
    double globalInterestTrend_ = 0.0;
};

} // namespace GPS
