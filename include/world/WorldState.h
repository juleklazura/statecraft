/**
 * GPS - Geopolitical Simulator
 * WorldState.h - Estado global do mundo
 * 
 * Contém todos os países, regiões e o estado global da simulação.
 * É o modelo de dados central acessado por todos os subsistemas.
 */
#pragma once

#include "core/Types.h"
#include <unordered_map>
#include <vector>
#include <string>

namespace GPS {

// ==================== Região / Província ====================
struct Region {
    RegionID id = INVALID_REGION;
    CountryID countryId = INVALID_COUNTRY;
    std::string name;
    std::string code;

    GeoCoord center;
    double areaSqKm = 0.0;
    double population = 0.0;

    // Economia regional
    double gdpBillions = 0.0;
    double unemploymentRate = 0.0;
    double avgIncome = 0.0;
    double industrialization = 0.0;

    // Infraestrutura
    double infrastructureLevel = 0.5;
    double healthcareAccess = 0.5;
    double educationLevel = 0.5;
    double internetPenetration = 0.5;

    // Recursos
    std::unordered_map<ResourceType, double> resources;

    // Social
    double crimeRate = 0.0;
    double satisfaction = 0.5;
    double urbanization = 0.5;
    double pollution = 0.0;

    // Construções
    std::vector<BuildingID> buildings;
};

// ==================== Setor Econômico ====================
struct EconomicSector {
    SectorID id;
    std::string name;
    double gdpContribution = 0.0;  // Em bilhões
    double employment = 0.0;       // Número de empregados
    double growthRate = 0.0;       // Taxa de crescimento anual
    double productivity = 1.0;
    double exportShare = 0.0;      // % exportada
    double importDependency = 0.0;
    double subsidies = 0.0;
    double taxBurden = 0.0;
    bool stateOwned = false;
    double regulation = 0.5;       // 0 = livre, 1 = regulação total

    // Dependências
    std::vector<SectorID> inputSectors;
    std::vector<ResourceType> requiredResources;
};

// ==================== Partido Político ====================
struct PoliticalParty {
    PartyID id;
    CountryID countryId;
    std::string name;
    std::string abbreviation;
    IdeologySpectrum ideology;

    double popularity = 0.0;       // 0-1
    double funding = 0.0;          // Em milhões
    double organization = 0.5;     // Capacidade organizacional
    double mediaPresence = 0.5;

    bool inPower = false;
    bool inCoalition = false;
    int seatsParliament = 0;

    // Posições políticas (0 = contra, 1 = a favor)
    double posEconomicFreedom = 0.5;
    double posSocialLiberty = 0.5;
    double posMilitarism = 0.5;
    double posEnvironment = 0.5;
    double posImmigration = 0.5;
    double posNationalism = 0.5;
    double posReligion = 0.5;
    double posWelfare = 0.5;
};

// ==================== Grupo Social ====================
struct SocialGroup {
    PopGroupID id;
    CountryID countryId;
    SocialGroupType type;
    std::string name;

    double populationShare = 0.0;    // % da população
    double satisfaction = 0.5;       // 0-1
    double politicalPower = 0.0;     // Influência política
    double radicalization = 0.0;     // 0-1, risco de revolta
    double governmentSupport = 0.5;  // Apoio ao governo

    // Interesses (importância de cada tema, 0-1)
    double interestEmployment = 0.5;
    double interestWages = 0.5;
    double interestTaxes = 0.5;
    double interestSecurity = 0.5;
    double interestFreedom = 0.5;
    double interestEnvironment = 0.3;
    double interestHealthcare = 0.5;
    double interestEducation = 0.5;

    // Modificadores ativos
    std::vector<Modifier> modifiers;
};

// ==================== Forças Armadas ====================
struct MilitaryForces {
    CountryID countryId;

    // Pessoal
    double activePersonnel = 0.0;
    double reservePersonnel = 0.0;
    double paramilitaryPersonnel = 0.0;

    // Equipamento
    int tanks = 0;
    int apc = 0;            // Veículos blindados
    int artillery = 0;
    int fighters = 0;       // Caças
    int bombers = 0;
    int helicopters = 0;
    int warships = 0;
    int submarines = 0;
    int carriers = 0;
    int missiles = 0;
    int drones = 0;

    // Nuclear
    int nuclearWarheads = 0;
    bool hasICBM = false;
    bool hasSubmarineLaunch = false;

    // Qualidade
    double training = 0.5;        // 0-1
    double morale = 0.5;
    double technology = 0.5;
    double readiness = 0.5;
    double experience = 0.5;
    double cyberCapability = 0.3;
    double spaceCapability = 0.1;

    // Orçamento
    Money annualBudget;
    double gdpShare = 0.02;       // % do PIB

    // Bases
    std::vector<BuildingID> bases;

    // Poder militar calculado
    double calculatePowerIndex() const {
        return (activePersonnel / 1000000.0) * 0.2 +
               (tanks / 10000.0) * 0.15 +
               (fighters / 1000.0) * 0.15 +
               (warships / 500.0) * 0.1 +
               (submarines / 100.0) * 0.1 +
               training * 0.1 +
               technology * 0.1 +
               (nuclearWarheads > 0 ? 0.1 : 0.0);
    }
};

// ==================== Relação Diplomática ====================
struct DiplomaticRelation {
    CountryID countryA;
    CountryID countryB;

    DiplomaticStatus status = DiplomaticStatus::NEUTRAL;
    double relations = 0.5;       // 0 = hostil, 1 = aliado
    double trust = 0.5;
    double tradeVolume = 0.0;     // Bilhões
    double influence = 0.0;       // A sobre B

    bool hasEmbassy = true;
    bool hasMilitaryAlliance = false;
    bool hasTradeAgreement = false;
    bool hasVisaFreeTravel = false;
    bool hasSanctions = false;
    bool hasArmsEmbargo = false;

    std::vector<TreatyID> treaties;
    std::vector<Modifier> modifiers;
};

// ==================== Estado do País ====================
struct Country {
    CountryID id = INVALID_COUNTRY;
    std::string name;
    std::string officialName;
    std::string isoCode;     // ISO 3166 alpha-3
    std::string capital;
    GeoCoord capitalCoord;
    std::string flagPath;

    // Geografia
    double areaSqKm = 0.0;
    double coastlineKm = 0.0;
    bool isLandlocked = false;
    std::vector<CountryID> neighbors;
    std::vector<RegionID> regions;

    // Governo
    GovernmentType government;
    EconomicSystem economicSystem;
    std::string headOfState;
    std::string headOfGovernment;
    int governmentStabilityDays = 0;
    double governmentApproval = 0.5;
    double corruption = 0.5;
    double ruleOfLaw = 0.5;
    double democracy = 0.5;
    double pressLiberty = 0.5;
    double institutionalTrust = 0.5;
    PartyID rulingPartyId = 0;
    std::vector<PartyID> coalitionParties;

    // População
    double population = 0.0;           // Em milhões
    double populationGrowthRate = 0.01;
    double birthRate = 0.02;
    double deathRate = 0.008;
    double lifeExpectancy = 72.0;
    double medianAge = 30.0;
    double urbanizationRate = 0.5;
    double literacyRate = 0.9;
    double hdi = 0.7;                  // Índice de Desenvolvimento Humano

    // Economia
    Money gdp;
    double gdpGrowthRate = 0.02;
    double gdpPerCapita = 0.0;
    double inflation = 0.03;
    double unemploymentRate = 0.05;
    double giniCoefficient = 0.4;    // Desigualdade
    double interestRate = 0.05;
    double exchangeRate = 1.0;        // vs USD
    Money publicDebt;
    double debtToGDP = 0.5;
    Money foreignReserves;
    Money tradeBalance;
    Money governmentBudget;
    Money governmentRevenue;
    Money governmentExpenditure;
    double taxBurden = 0.3;           // % do PIB

    // Impostos detalhados
    double incomeTaxLow = 0.10;
    double incomeTaxMid = 0.20;
    double incomeTaxHigh = 0.35;
    double corporateTax = 0.25;
    double salesTax = 0.10;
    double importTariff = 0.05;
    double exportTariff = 0.01;
    double carbonTax = 0.00;

    // Orçamento por setor (% do orçamento total)
    double budgetDefense = 0.10;
    double budgetEducation = 0.14;
    double budgetHealthcare = 0.12;
    double budgetInfrastructure = 0.07;
    double budgetSocialWelfare = 0.18;
    double budgetScience = 0.05;
    double budgetAdministration = 0.06;
    double budgetDebtService = 0.10;
    double budgetSecurity = 0.06;     // Segurança Pública
    double budgetAgriculture = 0.04;  // Agricultura
    double budgetCulture = 0.02;      // Mídia / Cultura
    double budgetTransport = 0.03;    // Transporte Público
    double budgetOther = 0.03;

    // Energia
    double energyProductionTWh = 0.0;
    double energyConsumptionTWh = 0.0;
    double renewableShare = 0.1;
    double oilProductionBarrelsDay = 0.0;
    double oilConsumptionBarrelsDay = 0.0;

    // Meio Ambiente
    double co2EmissionsMT = 0.0;       // Megatons
    double deforestationRate = 0.0;
    double airQualityIndex = 50.0;
    double waterQualityIndex = 0.7;
    double disasterRisk = 0.0;

    // Segurança / Inteligência
    double terrorismThreat = 0.0;
    double internalStability = 0.7;
    double borderSecurity = 0.5;
    double cyberDefense = 0.3;

    // Política Internacional
    double softPower = 0.5;
    double globalInfluence = 0.0;
    std::vector<OrganizationID> memberships;

    // Sistemas militares
    MilitaryForces military;

    // Social groups, parties, sectors
    std::vector<PopGroupID> socialGroups;
    std::vector<PartyID> parties;
    std::vector<SectorID> economicSectors;
    std::vector<LawID> activeLaws;

    // Modificadores ativos
    std::vector<Modifier> modifiers;

    // Helpers
    bool isPlayerControlled = false;
    bool isAIActive = true;

    double calculatePowerIndex() const {
        double eco = std::log10(std::max(1.0, gdp.billions)) / 5.0;
        double mil = military.calculatePowerIndex();
        double pop = std::log10(std::max(1.0, population)) / 4.0;
        double tech = hdi;
        double diplo = softPower;
        return (eco * 0.3 + mil * 0.25 + pop * 0.15 + tech * 0.15 + diplo * 0.15);
    }
};

// ==================== Tratado Internacional ====================
struct Treaty {
    TreatyID id;
    std::string name;
    std::string description;
    SimDate dateSignd;
    std::vector<CountryID> signatories;

    enum class Type {
        TRADE_AGREEMENT,
        MILITARY_ALLIANCE,
        NON_AGGRESSION,
        PEACE_TREATY,
        ARMS_CONTROL,
        ENVIRONMENTAL,
        EXTRADITION,
        CULTURAL_EXCHANGE,
        TECHNOLOGY_SHARING,
        MUTUAL_DEFENSE
    } type;

    bool active = true;
    int durationYears = -1; // -1 = indefinido
};

// ==================== Organização Internacional ====================
struct InternationalOrganization {
    OrganizationID id;
    std::string name;
    std::string abbreviation;
    std::vector<CountryID> members;
    CountryID headquarters;

    enum class OrgType {
        UNITED_NATIONS,
        MILITARY_ALLIANCE,
        TRADE_BLOC,
        REGIONAL,
        ECONOMIC,
        ENVIRONMENTAL,
        HUMANITARIAN
    } type;

    double influence = 0.0;
    double funding = 0.0;
    bool hasSecurityCouncil = false;
    std::vector<CountryID> securityCouncilPermanent;
};

// ==================== World State ====================
class WorldState {
public:
    WorldState() = default;

    // Gerenciamento de países
    void addCountry(Country country);
    Country& getCountry(CountryID id);
    const Country& getCountry(CountryID id) const;
    std::vector<CountryID> getAllCountryIds() const;
    int getCountryCount() const { return static_cast<int>(countries_.size()); }
    bool countryExists(CountryID id) const { return countries_.count(id) > 0; }

    // Gerenciamento de regiões
    void addRegion(Region region);
    Region& getRegion(RegionID id);
    const Region& getRegion(RegionID id) const;
    std::vector<RegionID> getRegionsOfCountry(CountryID id) const;

    // Relações diplomáticas
    DiplomaticRelation& getRelation(CountryID a, CountryID b);
    const DiplomaticRelation& getRelation(CountryID a, CountryID b) const;
    void setRelation(CountryID a, CountryID b, const DiplomaticRelation& rel);

    // Tratados
    void addTreaty(Treaty treaty);
    Treaty& getTreaty(TreatyID id);
    std::vector<TreatyID> getTreatiesOfCountry(CountryID id) const;

    // Organizações
    void addOrganization(InternationalOrganization org);
    InternationalOrganization& getOrganization(OrganizationID id);

    // Setores econômicos globais
    void addSector(EconomicSector sector);
    EconomicSector& getSector(SectorID id);

    // Partidos
    void addParty(PoliticalParty party);
    PoliticalParty& getParty(PartyID id);
    std::vector<PartyID> getPartiesOfCountry(CountryID id) const;

    // Grupos sociais
    void addSocialGroup(SocialGroup group);
    SocialGroup& getSocialGroup(PopGroupID id);
    std::vector<PopGroupID> getSocialGroupsOfCountry(CountryID id) const;

    // Indicadores globais
    double getGlobalGDP() const;
    double getGlobalPopulation() const;
    double getGlobalCO2() const;
    double getGlobalTemperatureAnomaly() const { return globalTempAnomaly_; }
    void setGlobalTemperatureAnomaly(double t) { globalTempAnomaly_ = t; }

    // Player
    void setPlayerCountry(CountryID id) { playerCountryId_ = id; }
    CountryID getPlayerCountry() const { return playerCountryId_; }

private:
    std::unordered_map<CountryID, Country> countries_;
    std::unordered_map<RegionID, Region> regions_;
    std::unordered_map<SectorID, EconomicSector> sectors_;
    std::unordered_map<PartyID, PoliticalParty> parties_;
    std::unordered_map<PopGroupID, SocialGroup> socialGroups_;
    std::unordered_map<TreatyID, Treaty> treaties_;
    std::unordered_map<OrganizationID, InternationalOrganization> organizations_;

    // Chave de relação: min(a,b) * 65536 + max(a,b)
    std::unordered_map<uint32_t, DiplomaticRelation> relations_;
    uint32_t makeRelationKey(CountryID a, CountryID b) const {
        auto mn = std::min(a, b);
        auto mx = std::max(a, b);
        return static_cast<uint32_t>(mn) * 65536 + mx;
    }

    CountryID playerCountryId_ = INVALID_COUNTRY;
    double globalTempAnomaly_ = 1.1; // °C acima da era pré-industrial
};

} // namespace GPS
