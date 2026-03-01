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

// Fase do ciclo econômico (expansão→pico→contração→recessão→fundo→recuperação)
enum class EconomicCyclePhase {
    EXPANSION,    // Crescimento acima de 3%, pleno emprego se aproximando
    PEAK,         // Sobreaquecimento: inflação alta, PIB no topo, juros subindo
    CONTRACTION,  // Desaceleração: crescimento positivo mas caindo
    RECESSION,    // Dois ou mais trimestres de crescimento negativo
    TROUGH,       // Fundo do ciclo: desemprego máximo, mercados deprimidos
    RECOVERY      // Saída da recessão: crescimento voltando, confiança retornando
};

// Indicadores de Investimento Estrangeiro Direto (IED)
struct FDIIndicators {
    double inflowsBillions = 0.0;       // Entrada de IED por ano
    double outflowsBillions = 0.0;      // Saída de IED por ano
    double stockBillions = 0.0;         // Estoque total de IED acumulado
    double fdiToGdpRatio = 0.0;         // IED / PIB
    double easeOfDoingBusiness = 0.5;   // 0=péssimo ambiente, 1=excelente ambiente negócios
    double politicalRiskPremium = 0.0;  // Prêmio de risco adicional por instabilidade (bps)
    double reinvestedEarnings = 0.0;    // Lucros reinvestidos por multinacionais
};

// Saúde do Sistema Bancário
struct BankingIndicators {
    double nonPerformingLoanRatio = 0.05; // Inadimplência (% da carteira)
    double capitalAdequacyRatio = 0.12;   // Índice de Basileia mínimo (recomendado 8%)
    double creditToGdp = 0.70;            // Crédito privado total / PIB
    double privateCreditGrowth = 0.05;    // Crescimento anual do crédito
    double bankingCrisisRisk = 0.0;       // 0-1 risco de crise sistêmica
    bool isBankingCrisis = false;         // Se há crise bancária ativa
    double centralBankIndependence = 0.7; // 0=política monetária submissa, 1=totalmente autônomo
    double loanToDepositRatio = 0.85;     // Alavancagem bancária
    double interbankRate = 0.03;          // Taxa interbancária overnight
};

// Composição Setorial do PIB e Produtividade
struct SectoralBreakdown {
    // Participação de cada setor no PIB (devem somar próximo de 1.0)
    double agriculture = 0.05;
    double industry = 0.25;
    double services = 0.55;
    double technology = 0.08;
    double energy = 0.04;
    double finance = 0.03;

    // Produtividade setorial relativa (1.0 = média nacional)
    double agricultureProductivity = 0.8;
    double industryProductivity = 1.0;
    double servicesProductivity = 1.0;
    double technologyProductivity = 1.6;
    double energyProductivity = 1.2;
    double financeProductivity = 1.4;

    // Crescimento anual de cada setor
    double agricultureGrowth = 0.02;
    double industryGrowth = 0.03;
    double servicesGrowth = 0.04;
    double technologyGrowth = 0.07;
    double energyGrowth = 0.03;
    double financeGrowth = 0.04;

    // Balança Comercial Setorial (superávit = positivo)
    double agriTradeBalance = 0.0;
    double industryTradeBalance = 0.0;
    double techTradeBalance = 0.0;
    double energyTradeBalance = 0.0;
};

// Economia Informal / Sombra
struct ShadowEconomy {
    double ratioToGdp = 0.15;          // Tamanho do setor informal em % do PIB
    double taxGapBillions = 0.0;        // Impostos não coletados por informalidade
    double formalizationTrend = 0.0;   // +positive = formalização crescendo; -negative = regressão
    double informelEmploymentRatio = 0.30; // % dos trabalhadores no setor informal
};

// Distribuição de Renda e Pobreza
struct IncomeDistribution {
    double povertyRate = 0.20;           // % da população abaixo da linha de pobreza
    double extremePovertyRate = 0.05;    // % em pobreza extrema (<US$2.15/dia)
    double incomeShareTop10 = 0.45;      // Participação dos 10% mais ricos na renda
    double incomeShareBottom50 = 0.15;   // Participação dos 50% mais pobres
    double socialMobilityIndex = 0.40;   // 0=estratificação absoluta, 1=mobilidade total
    double middleClassShare = 0.40;      // % da população na classe média
};

// Fundo Soberano / Reservas Estratégicas
struct SovereignWealthData {
    double fundBillions = 0.0;          // Ativos do fundo soberano
    double annualContribution = 0.0;    // Contribuição anual (% da receita de royalties)
    bool isActive = false;
    double targetFundSize = 0.0;        // Meta de tamanho do fundo
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

    // Ações do jogador — Política Fiscal e Monetária
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
    void setSpendingRatio(CountryID country, double gdpPct); // 0.15-0.55 como % do PIB

    // Ações do jogador — Novas políticas econômicas
    void activateEconomicStimulus(CountryID country, double amountBillions);
    void implementAusterityPackage(CountryID country, double cutRatio);
    void establishSovereignWealthFund(CountryID country, double initialBillions, double annualContribRate);
    void setCentralBankIndependence(CountryID country, double level);
    void bankBailout(CountryID country, double amountBillions);
    void formalizeShadowEconomy(CountryID country, double policyStrength);

    // Consultas — Básicas
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

    // Consultas — Novas
    EconomicCyclePhase getEconomicCyclePhase(CountryID country) const;
    FDIIndicators getFDIIndicators(CountryID country) const;
    BankingIndicators getBankingIndicators(CountryID country) const;
    SectoralBreakdown getSectoralBreakdown(CountryID country) const;
    ShadowEconomy getShadowEconomy(CountryID country) const;
    IncomeDistribution getIncomeDistribution(CountryID country) const;
    SovereignWealthData getSovereignWealthData(CountryID country) const;
    double getPovertyRate(CountryID country) const;
    double getProductivityGrowth(CountryID country) const;
    double getBusinessConfidenceIndex(CountryID country) const;
    double getLafferOptimalTax(CountryID country) const;
    std::string getEconomicCycleName(CountryID country) const;

private:
    void updateCountryEconomy(Country& country, double dt);
    void updateGlobalTrade(double dt);
    void updateMarkets(double dt);
    void recalculateSectors(Country& country);
    void processMonetaryPolicy(Country& country);
    void processFiscalPolicy(Country& country);
    double calculateSectorGrowth(const Country& country, const EconomicSector& sector);

    // Novos subsistemas
    void updateBankingSector(Country& country, double dt);
    void updateFDIFlows(Country& country, double dt);
    void updateSectoralBreakdown(Country& country, double dt);
    void updateShadowEconomy(Country& country, double dt);
    void updateEconomicCycle(Country& country, double dt);
    void updateIncomeDistribution(Country& country, double dt);
    void updateSovereignWealthFund(Country& country, double dt);
    double applyLafferCurveCorrection(double nominalTaxRevenue, double weightedTaxRate,
                                       double optimalRate, double taxEfficiency);
    double computeProductivityFactor(const Country& country) const;
    double computeBusinessConfidence(const Country& country) const;

    WorldState& world_;
    const SimulationConfig& config_;

    std::unordered_map<CountryID, TaxPolicy> taxPolicies_;
    std::unordered_map<CountryID, BudgetAllocation> budgets_;
    std::unordered_map<CountryID, MarketIndicators> markets_;
    std::unordered_map<CountryID, std::vector<TradeData>> trades_;

    // Novos dados econômicos
    std::unordered_map<CountryID, EconomicCyclePhase> economicCycles_;
    std::unordered_map<CountryID, FDIIndicators> fdiData_;
    std::unordered_map<CountryID, BankingIndicators> bankingData_;
    std::unordered_map<CountryID, SectoralBreakdown> sectors_;
    std::unordered_map<CountryID, ShadowEconomy> shadowEconomies_;
    std::unordered_map<CountryID, IncomeDistribution> incomeDistributions_;
    std::unordered_map<CountryID, SovereignWealthData> sovereignFunds_;
    std::unordered_map<CountryID, double> productivityGrowth_;
    std::unordered_map<CountryID, double> businessConfidence_;
    std::unordered_map<CountryID, int> recessionQuarters_;

    // Preços globais de commodities
    double globalOilPrice_ = 70.0;
    double globalGoldPrice_ = 1800.0;
    double globalInterestTrend_ = 0.0;

    // Índices globais de estresse financeiro
    double globalCommodityIndex_ = 100.0;   // 100 = baseline
    double globalFinancialStressIndex_ = 0.0; // 0=calmo, 1=crise sistêmica
    double globalSupplyChainPressure_ = 0.0;  // Perturbações logísticas globais
};

} // namespace GPS
