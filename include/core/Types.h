/**
 * GPS - Geopolitical Simulator
 * Types.h - Tipos fundamentais do simulador
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include <functional>
#include <optional>
#include <variant>
#include <array>
#include <chrono>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

namespace GPS {

// ==================== IDs ====================
using CountryID = uint16_t;
using RegionID = uint32_t;
using LawID = uint32_t;
using EventID = uint32_t;
using TreatyID = uint32_t;
using UnitID = uint32_t;
using BuildingID = uint32_t;
using SectorID = uint16_t;
using PopGroupID = uint32_t;
using PartyID = uint16_t;
using OperationID = uint32_t;
using OrganizationID = uint16_t;

constexpr CountryID INVALID_COUNTRY = 0xFFFF;
constexpr RegionID INVALID_REGION = 0xFFFFFFFF;

// ==================== Tempo de Simulação ====================
struct SimDate {
    uint16_t year = 2026;
    uint8_t month = 1;
    uint8_t day = 1;
    uint8_t hour = 0;

    int totalDays() const {
        return year * 365 + month * 30 + day;
    }

    bool operator<(const SimDate& other) const { return totalDays() < other.totalDays(); }
    bool operator>(const SimDate& other) const { return totalDays() > other.totalDays(); }
    bool operator==(const SimDate& other) const { return totalDays() == other.totalDays(); }
    bool operator<=(const SimDate& other) const { return totalDays() <= other.totalDays(); }
    bool operator>=(const SimDate& other) const { return totalDays() >= other.totalDays(); }

    void advanceDay() {
        day++;
        if (day > 30) { day = 1; month++; }
        if (month > 12) { month = 1; year++; }
    }

    void advanceHour() {
        hour++;
        if (hour >= 24) { hour = 0; advanceDay(); }
    }

    std::string toString() const {
        std::ostringstream ss;
        ss << year << "-" << (int)month << "-" << (int)day;
        return ss.str();
    }
};

// ==================== Enums Fundamentais ====================
enum class GameSpeed : uint8_t {
    PAUSED = 0,
    SLOW = 1,        // 1 hora de jogo / segundo real
    NORMAL = 2,      // 6 horas / segundo
    FAST = 3,        // 1 dia / segundo
    VERY_FAST = 4,   // 1 semana / segundo
    ULTRA_FAST = 5   // 1 mês / segundo
};

enum class Difficulty : uint8_t {
    EASY = 0,
    NORMAL = 1,
    HARD = 2,
    REALISTIC = 3,
    NIGHTMARE = 4
};

enum class GovernmentType : uint8_t {
    PRESIDENTIAL_REPUBLIC,
    PARLIAMENTARY_REPUBLIC,
    SEMI_PRESIDENTIAL,
    CONSTITUTIONAL_MONARCHY,
    ABSOLUTE_MONARCHY,
    THEOCRACY,
    ONE_PARTY_STATE,
    MILITARY_JUNTA,
    FEDERATION,
    CONFEDERATION,
    DIRECT_DEMOCRACY,
    AUTHORITARIAN_REGIME,
    TOTALITARIAN_REGIME,
    TRANSITIONAL_GOVERNMENT,
    FAILED_STATE
};

enum class IdeologySpectrum : uint8_t {
    FAR_LEFT,
    LEFT,
    CENTER_LEFT,
    CENTER,
    CENTER_RIGHT,
    RIGHT,
    FAR_RIGHT
};

enum class EconomicSystem : uint8_t {
    FREE_MARKET,
    MIXED_ECONOMY,
    SOCIAL_MARKET,
    STATE_CAPITALISM,
    PLANNED_ECONOMY,
    SOCIALIST,
    COMMUNIST
};

enum class DiplomaticStatus : uint8_t {
    ALLIED,
    FRIENDLY,
    NEUTRAL,
    COOL,
    HOSTILE,
    AT_WAR,
    EMBARGO,
    SANCTIONS
};

enum class MilitaryBranch : uint8_t {
    ARMY,
    NAVY,
    AIR_FORCE,
    MARINES,
    SPECIAL_FORCES,
    CYBER_COMMAND,
    SPACE_FORCE,
    NUCLEAR_FORCES,
    COAST_GUARD,
    NATIONAL_GUARD
};

enum class ResourceType : uint8_t {
    OIL,
    NATURAL_GAS,
    COAL,
    IRON,
    COPPER,
    GOLD,
    SILVER,
    URANIUM,
    LITHIUM,
    RARE_EARTHS,
    DIAMONDS,
    TIMBER,
    FISH,
    WATER,
    ARABLE_LAND,
    SILICON,
    ALUMINUM,
    TITANIUM,
    COBALT,
    WHEAT,
    RICE,
    CORN,
    SOYBEANS,
    COFFEE,
    RUBBER
};

enum class SocialGroupType : uint8_t {
    LOWER_CLASS,
    LOWER_MIDDLE_CLASS,
    MIDDLE_CLASS,
    UPPER_MIDDLE_CLASS,
    UPPER_CLASS,
    WORKERS,
    ENTREPRENEURS,
    PUBLIC_SERVANTS,
    MILITARY_PERSONNEL,
    MILITARY = MILITARY_PERSONNEL,
    STUDENTS,
    RETIREES,
    UNEMPLOYED,
    FARMERS,
    INTELLECTUALS,
    RELIGIOUS_GROUPS,
    RELIGIOUS = RELIGIOUS_GROUPS,
    ETHNIC_MINORITIES,
    MINORITIES = ETHNIC_MINORITIES,
    IMMIGRANTS,
    YOUTH,
    ELDERLY,
    UNIONS,
    WOMEN,
    ENVIRONMENTALISTS,
    INDIGENOUS,
    JOURNALISTS,
    ACTIVISTS,
    NATIONALISTS,
    TEACHERS,
    SCIENTISTS
};

enum class LawCategory : uint8_t {
    ECONOMIC,
    LABOR,
    PENSION,
    ENVIRONMENTAL,
    EDUCATION,
    CRIMINAL,
    IMMIGRATION,
    PUBLIC_SECURITY,
    CIVIL_RIGHTS,
    HEALTHCARE,
    TAX,
    TRADE,
    MILITARY,
    MEDIA,
    TECHNOLOGY,
    HOUSING,
    TRANSPORTATION,
    ENERGY,
    AGRICULTURE,
    FOREIGN_AFFAIRS
};

enum class CrisisType : uint8_t {
    FINANCIAL_CRASH,
    EPIDEMIC,
    PANDEMIC,
    NATURAL_DISASTER,
    POLITICAL_CRISIS,
    MILITARY_CONFLICT,
    TERRORISM,
    CYBER_ATTACK,
    REFUGEE_CRISIS,
    FAMINE,
    ENVIRONMENTAL_DISASTER,
    SOCIAL_UNREST,
    COUP_ATTEMPT,
    CIVIL_WAR,
    NUCLEAR_INCIDENT,
    TRADE_WAR,
    ENERGY_CRISIS,
    DIPLOMATIC_INCIDENT
};

enum class InfrastructureType : uint8_t {
    HIGHWAY,
    RAILROAD,
    PORT,
    AIRPORT,
    POWER_PLANT_COAL,
    POWER_PLANT_GAS,
    POWER_PLANT_NUCLEAR,
    POWER_PLANT_SOLAR,
    POWER_PLANT_WIND,
    POWER_PLANT_HYDRO,
    HOSPITAL,
    UNIVERSITY,
    SCHOOL,
    MILITARY_BASE,
    NAVAL_BASE,
    AIR_BASE,
    RESEARCH_CENTER,
    FACTORY,
    REFINERY,
    PIPELINE,
    TELECOM_NETWORK,
    WATER_TREATMENT,
    DAM,
    SPACE_CENTER
};

// ==================== Structs Utilitários ====================

// Intervalo normalizado 0.0 - 1.0
struct Percentage {
    double value = 0.0;

    Percentage() = default;
    explicit Percentage(double v) : value(std::clamp(v, 0.0, 1.0)) {}

    operator double() const { return value; }
    Percentage& operator+=(double d) { value = std::clamp(value + d, 0.0, 1.0); return *this; }
    Percentage& operator-=(double d) { value = std::clamp(value - d, 0.0, 1.0); return *this; }
};

// Valor monetário em bilhões de USD
struct Money {
    double billions = 0.0;

    Money() = default;
    explicit Money(double b) : billions(b) {}

    Money operator+(const Money& o) const { return Money(billions + o.billions); }
    Money operator-(const Money& o) const { return Money(billions - o.billions); }
    Money operator*(double factor) const { return Money(billions * factor); }
    Money& operator+=(const Money& o) { billions += o.billions; return *this; }
    Money& operator-=(const Money& o) { billions -= o.billions; return *this; }
    bool operator<(const Money& o) const { return billions < o.billions; }
    bool operator>(const Money& o) const { return billions > o.billions; }
};

// Coordenada geográfica
struct GeoCoord {
    float latitude = 0.0f;
    float longitude = 0.0f;
};

// Modificador temporário com decaimento
struct Modifier {
    std::string name;
    std::string source;
    double magnitude = 0.0;    // -1.0 a +1.0
    int daysRemaining = -1;    // -1 = permanente
    bool stackable = false;

    void decay() { if (daysRemaining > 0) daysRemaining--; }
    bool expired() const { return daysRemaining == 0; }
};

// Resultado genérico de callback de sistema
using SystemCallback = std::function<void()>;
using EventCallback = std::function<void(EventID, CountryID)>;

// Pool de números aleatórios
class RandomEngine {
public:
    static RandomEngine& instance() {
        static RandomEngine eng;
        return eng;
    }

    int randInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng_);
    }

    double randDouble(double min, double max) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(rng_);
    }

    double randNormal(double mean, double stddev) {
        std::normal_distribution<double> dist(mean, stddev);
        return dist(rng_);
    }

    bool chance(double probability) {
        return randDouble(0.0, 1.0) < probability;
    }

    void seed(uint64_t s) { rng_.seed(s); }

private:
    RandomEngine() : rng_(std::random_device{}()) {}
    std::mt19937_64 rng_;
};

} // namespace GPS
