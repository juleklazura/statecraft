/**
 * GPS - Geopolitical Simulator
 * GameUI.cpp - Interface FTXUI completa
 */

#include "ui/GameUI.h"
#include "systems/economy/EconomySystem.h"
#include "systems/diplomacy/DiplomacySystem.h"
#include "systems/military/MilitarySystem.h"
#include "systems/intelligence/IntelligenceSystem.h"
#include "systems/laws/LawSystem.h"
#include "systems/politics/PoliticsSystem.h"
#include "systems/population/PopulationSystem.h"
#include "systems/events/EventSystem.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/event.hpp>

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace GPS {

using namespace ftxui;

// ============================================================
// Formatting helpers
// ============================================================

std::string GameUI::fmtMoney(double billions) const {
    std::ostringstream ss;
    if (std::abs(billions) >= 1000.0) {
        ss << std::fixed << std::setprecision(1) << (billions / 1000.0) << "T";
    } else {
        ss << std::fixed << std::setprecision(1) << billions << "B";
    }
    return ss.str();
}

std::string GameUI::fmtPct(double ratio) const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << (ratio * 100.0) << "%";
    return ss.str();
}

std::string GameUI::fmtNum(double num) const {
    std::ostringstream ss;
    if (num >= 1e6) ss << std::fixed << std::setprecision(1) << (num / 1e6) << "M";
    else if (num >= 1e3) ss << std::fixed << std::setprecision(0) << (num / 1e3) << "K";
    else ss << std::fixed << std::setprecision(0) << num;
    return ss.str();
}

std::string GameUI::fmtDate(const SimDate& date) const {
    std::ostringstream ss;
    ss << date.year << "/"
       << std::setw(2) << std::setfill('0') << (int)date.month << "/"
       << std::setw(2) << std::setfill('0') << (int)date.day;
    return ss.str();
}

std::string GameUI::fmtPop(double millions) const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << millions << "M";
    return ss.str();
}

// ============================================================
// Rendering helpers
// ============================================================

Element GameUI::renderStatBox(const std::string& label, const std::string& value, Color c) {
    return vbox({
        text(" " + label) | dim,
        text(" " + value + " ") | bold | color(c),
    }) | border | size(WIDTH, EQUAL, 19);
}

Element GameUI::renderGaugeBar(const std::string& label, double value, Color c, int labelWidth) {
    value = std::clamp(value, 0.0, 1.0);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << (value * 100.0) << "%";
    // Escolhe ícone de nível
    std::string icon;
    if      (value >= 0.75) icon = "█";
    else if (value >= 0.50) icon = "▓";
    else if (value >= 0.25) icon = "▒";
    else                    icon = "░";
    return hbox({
        text(" ") ,
        text(label) | size(WIDTH, EQUAL, labelWidth),
        gauge(value) | color(c) | flex,
        text(" " + ss.str()) | bold | color(c) | size(WIDTH, EQUAL, 8),
    });
}

Element GameUI::renderDualGauge(const std::string& label, double value,
                                 Color posColor, Color negColor) {
    std::ostringstream ss;
    ss << std::showpos << std::fixed << std::setprecision(1) << (value * 100.0) << "%";
    Color c = value >= 0 ? posColor : negColor;
    return hbox({
        text(label) | size(WIDTH, EQUAL, 16),
        text(ss.str()) | bold | color(c),
    });
}

// Renderiza uma linha de fator de aprovação (ex: "↑ Crescimento PIB: +2.3%")
static Element renderApprovalFactor(const std::string& label, double impact) {
    std::ostringstream ss;
    ss << std::showpos << std::fixed << std::setprecision(1) << (impact * 100.0) << "pp";
    bool positive = impact >= 0.005;
    bool negative  = impact <= -0.005;
    Color c = positive ? Color::GreenLight : (negative ? Color::RedLight : Color::GrayLight);
    std::string arrow = positive ? "▲" : (negative ? "▼" : "─");
    return hbox({
        text(" "),
        text(arrow) | bold | color(c) | size(WIDTH, EQUAL, 2),
        text(label) | size(WIDTH, EQUAL, 22),
        text(ss.str()) | bold | color(c) | size(WIDTH, EQUAL, 9),
    });
}

Element GameUI::renderHeader() {
    auto& c = engine_.getWorld().getCountry(playerId_);
    auto date = engine_.getCurrentDate();

    Color approvalColor = c.governmentApproval > 0.60 ? Color::GreenLight :
                          c.governmentApproval > 0.40 ? Color::Yellow : Color::RedLight;
    Color growthColor   = c.gdpGrowthRate >= 0 ? Color::GreenLight : Color::RedLight;
    std::string growthArrow = c.gdpGrowthRate >= 0 ? "▲" : "▼";
    std::string approvalIcon = c.governmentApproval > 0.60 ? "✔" :
                               c.governmentApproval > 0.40 ? "●" : "✖";

    double fb = c.governmentBudget.billions;
    Color balColor  = fb >= 0 ? Color::GreenLight : Color::RedLight;
    std::string balStr = fb >= 0 ? ("+$" + fmtMoney(fb) + " ▲") : ("-$" + fmtMoney(-fb) + " ▼");

    return hbox({
        text(" ◆ GPS ") | bold | color(Color::White) | bgcolor(Color::Blue),
        text(" "),
        text(c.name) | bold | color(Color::Cyan),
        text(" "),
        separator(),
        text(" " + fmtDate(date) + " ") | bold | color(Color::Yellow),
        separator(),
        text(" Pop: ") | dim,
        text(fmtPop(c.population)) | color(Color::White),
        text("  PIB: ") | dim,
        text("$" + fmtMoney(c.gdp.billions)) | color(Color::GreenLight),
        text("  " + growthArrow + fmtPct(c.gdpGrowthRate)) | color(growthColor),
        text("  Infl: ") | dim,
        text(fmtPct(c.inflation)) | color(c.inflation > 0.06 ? Color::RedLight : Color::Yellow),
        text("  Aprov: ") | dim,
        text(approvalIcon + " " + fmtPct(c.governmentApproval)) | bold | color(approvalColor),
        text("  Saldo: ") | dim,
        text(balStr) | bold | color(balColor),
        filler(),
        text(" " + std::to_string(engine_.getTotalTicksProcessed()) + " ciclos ") | dim,
    });
}

Element GameUI::renderStatusBar() {
    std::string statusText;
    Color notifColor = Color::White;
    if (!notifications_.empty()) {
        statusText = notifications_.back();
        // Colorir por tipo
        if (statusText.find("✓") != std::string::npos || statusText.find("\xe2\x9c\x94") != std::string::npos)
            notifColor = Color::GreenLight;
        else if (statusText.find("✗") != std::string::npos || statusText.find("GUERRA") != std::string::npos)
            notifColor = Color::RedLight;
        else if (statusText.find("⚠") != std::string::npos || statusText.find("🏦") != std::string::npos)
            notifColor = Color::Yellow;
    } else {
        statusText = " ◀ W=semana  M=mês  Y=ano  Q=sair  │  Tab/Setas=navegar  Enter=selecionar";
    }
    return hbox({
        text(" "),
        text(statusText) | color(notifColor) | flex,
        text(" │ GPS Geopolitical Simulator v0.1 ") | dim,
    }) | bgcolor(Color::GrayDark);
}

void GameUI::addNotification(const std::string& msg) {
    notifications_.push_back(msg);
    if (notifications_.size() > 20) {
        notifications_.erase(notifications_.begin());
    }
}

// ============================================================
// Constructor
// ============================================================

GameUI::GameUI(SimulationEngine& engine)
    : engine_(engine)
{
    playerId_ = engine_.getWorld().getPlayerCountry();

    tabNames_ = {
        " \u25a3 Painel    ",
        " \u20bf Economia  ",
        " \u2694 Diplomacia",
        " \u2605 Militar   ",
        " \u2261 Leis      ",
        " \u25ce Intel.    ",
        " \u2630 Ranking   ",
    };

    lawCategoryNames_ = {
        "Tributária",      "Econômica",       "Trabalhista",
        "Previdência",     "Ambiental",       "Educação",
        "Criminal",        "Imigração",       "Seg. Pública",
        "Dir. Civis",      "Saúde",           "Comércio",
        "Militar",         "Mídia",           "Tecnologia",
        "Habitação",       "Transporte",      "Energia",
        "Agricultura",     "Rel. Exteriores",
    };

    // Sync tax/budget sliders from country data
    auto& c = engine_.getWorld().getCountry(playerId_);
    taxLow_ = (int)(c.incomeTaxLow * 100);
    taxMid_ = (int)(c.incomeTaxMid * 100);
    taxHigh_ = (int)(c.incomeTaxHigh * 100);
    taxCorp_ = (int)(c.corporateTax * 100);
    taxSales_ = (int)(c.salesTax * 100);
    taxImport_ = (int)(c.importTariff * 100);
    budDefense_     = (int)(c.budgetDefense      * 100);
    budEducation_   = (int)(c.budgetEducation    * 100);
    budHealthcare_  = (int)(c.budgetHealthcare   * 100);
    budInfra_       = (int)(c.budgetInfrastructure * 100);
    budSocial_      = (int)(c.budgetSocialWelfare * 100);
    budScience_     = (int)(c.budgetScience      * 100);
    budEnv_         = (int)(c.budgetOther        * 100);  // fallback
    budAdmin_       = (int)(c.budgetAdministration * 100);
    budDebt_        = (int)(c.budgetDebtService  * 100);
    budSecurity_    = 6;
    budAgriculture_ = 4;
    budCulture_     = 2;
    budTransport_   = 3;
    auto ba0 = engine_.getEconomy().getBudgetAllocation(playerId_);
    spendingRatio_ = (int)(ba0.totalGdpRatio * 100);

    ecoSubTabNames_ = {
        " Panorama Geral ",
        " Impostos       ",
        " Gastos Gov.    ",
        " Defesa/Militar ",
        " Educação       ",
        " Saúde          ",
        " Previdência    ",
        " Ciência/Tec.   ",
        " Infraestrutura ",
        " Transporte     ",
        " Amb./Energia   ",
        " Seg. Pública   ",
        " Agricultura    ",
        " Mídia/Cultura  ",
        " Administração  ",
        " Serviço Dívida ",
    };

    updateLawTemplateNames();
}

void GameUI::updateLawTemplateNames() {
    LawCategory cats[] = {
        LawCategory::TAX, LawCategory::ECONOMIC, LawCategory::LABOR,
        LawCategory::PENSION, LawCategory::ENVIRONMENTAL, LawCategory::EDUCATION,
        LawCategory::CRIMINAL, LawCategory::IMMIGRATION, LawCategory::PUBLIC_SECURITY,
        LawCategory::CIVIL_RIGHTS, LawCategory::HEALTHCARE, LawCategory::TRADE,
        LawCategory::MILITARY, LawCategory::MEDIA, LawCategory::TECHNOLOGY,
        LawCategory::HOUSING, LawCategory::TRANSPORTATION, LawCategory::ENERGY,
        LawCategory::AGRICULTURE, LawCategory::FOREIGN_AFFAIRS,
    };

    int idx = std::clamp(lawCategoryIdx_, 0, 19);
    auto templates = engine_.getLaws().getAvailableLawTemplates(cats[idx], playerId_);

    lawTemplateNames_.clear();
    for (auto& t : templates) {
        // Indicar o motivo do bloqueio com precisão
        std::string reason = engine_.getLaws().getLawBlockReason(playerId_, t.name);
        lawTemplateNames_.push_back(reason.empty() ? t.name : (reason + " " + t.name));
    }
    if (lawTemplateIdx_ >= (int)lawTemplateNames_.size()) {
        lawTemplateIdx_ = 0;
    }
}

// ============================================================
// DASHBOARD TAB
// ============================================================

Component GameUI::createDashboardTab() {
    return Renderer([&] {
        auto& c = engine_.getWorld().getCountry(playerId_);

        // ── Índice de criminalidade estimado (proxy)
        double crimeIndex = std::clamp(
            (1.0 - c.ruleOfLaw) * 0.45 + c.corruption * 0.35 + c.terrorismThreat * 0.20,
            0.0, 1.0);

        // ── Fatores que afetam a aprovação ──────────────────────
        double growthImpact    = std::clamp((c.gdpGrowthRate - 0.02) * 4.0,  -0.25, 0.25);
        double inflationImpact = std::clamp(-(c.inflation - 0.035) * 3.5,    -0.30, 0.15);
        double unemployImpact  = std::clamp(-(c.unemploymentRate - 0.05) * 5.0, -0.30, 0.10);
        double stabilityImpact = std::clamp((c.internalStability - 0.5) * 0.7,  -0.20, 0.20);
        double corruptImpact   = std::clamp(-(c.corruption - 0.30) * 0.8,   -0.20, 0.10);
        double debtImpact      = std::clamp(-(c.debtToGDP - 0.50) * 0.6,    -0.15, 0.10);
        double balanceImpact   = [&] {
            double fb = c.governmentBudget.billions;
            if (fb > 0)  return std::clamp(fb / c.gdp.billions * 2.0, 0.0, 0.10);
            else         return std::clamp(fb / c.gdp.billions * 3.0, -0.20, 0.0);
        }();

        // Impacto das leis aprovadas (últimas 3 ativas)
        auto activeLaws = engine_.getLaws().getActiveLaws(playerId_);
        double lawsApprovalNet = 0.0;
        std::vector<std::pair<std::string,double>> lawImpacts;
        for (int i = (int)activeLaws.size()-1; i >= 0 && (int)lawImpacts.size() < 3; --i) {
            double imp = activeLaws[i].shortTermEffects.happiness * 0.5
                       + activeLaws[i].longTermEffects.happiness  * 0.5;
            if (std::abs(imp) >= 0.001) {
                lawsApprovalNet += imp;
                lawImpacts.push_back({activeLaws[i].name, imp});
            }
        }
        double lawImpactClamped = std::clamp(lawsApprovalNet, -0.25, 0.25);

        // ── SEÇÃO APROVAÇÃO ─────────────────────────────────────
        Color approvalColor = c.governmentApproval > 0.60 ? Color::Green :
                              c.governmentApproval > 0.40 ? Color::Yellow : Color::Red;

        Elements approvalFactors;
        approvalFactors.push_back(renderApprovalFactor("Cresc. PIB",           growthImpact));
        approvalFactors.push_back(renderApprovalFactor("Inflação",             inflationImpact));
        approvalFactors.push_back(renderApprovalFactor("Desemprego",           unemployImpact));
        approvalFactors.push_back(renderApprovalFactor("Estabilidade",         stabilityImpact));
        approvalFactors.push_back(renderApprovalFactor("Corrupção",            corruptImpact));
        approvalFactors.push_back(renderApprovalFactor("Dívida Pública",       debtImpact));
        approvalFactors.push_back(renderApprovalFactor("Balanço Fiscal",       balanceImpact));
        if (!lawImpacts.empty())
            approvalFactors.push_back(renderApprovalFactor("Leis Aprovadas",   lawImpactClamped));

        auto approvalSection = vbox({
            text("╭────────── ● APROVAÇÃO DO GOVERNO ● ─────────╮") | bold | color(approvalColor),
            hbox({
                gauge(c.governmentApproval) | color(approvalColor) | flex,
                text(" " + fmtPct(c.governmentApproval)) | bold | color(approvalColor)
                    | size(WIDTH, EQUAL, 7),
            }),
            separator(),
            text("  Fatores de Impacto:") | dim,
            vbox(approvalFactors),
            separator(),
            text("  Confiança Institucional: " + fmtPct(c.institutionalTrust)) | dim,
            [&]() -> Element {
                if (lawImpacts.empty()) return text("") | dim;
                Elements lawLines;
                lawLines.push_back(text("  Leis recentes:") | dim);
                for (auto& [name, imp] : lawImpacts) {
                    std::ostringstream ss;
                    ss << std::showpos << std::fixed << std::setprecision(1) << (imp*100) << "pp";
                    Color lc = imp >= 0 ? Color::GreenLight : Color::RedLight;
                    lawLines.push_back(hbox({
                        text("    • ") | color(lc),
                        text(name.substr(0,26)) | size(WIDTH, EQUAL, 27),
                        text(ss.str()) | bold | color(lc),
                    }));
                }
                return vbox(lawLines);
            }(),
        }) | border;

        // ── SEÇÃO ECONOMIA ──────────────────────────────────────
        double fb = c.governmentBudget.billions;
        Color balColor = fb >= 0 ? Color::GreenLight : Color::RedLight;
        std::string balLabel = fb >= 0
            ? "Superávit +$" + fmtMoney(fb)
            : "Déficit  -$"  + fmtMoney(-fb);

        auto econSection = vbox({
            text("╭───────────── ₿ ECONOMIA ─────────────╮") | bold | color(Color::GreenLight),
            hbox({
                renderStatBox("PIB",       "$" + fmtMoney(c.gdp.billions),  Color::GreenLight),
                renderStatBox("PIB/cap",   "$" + fmtNum(c.gdpPerCapita),    Color::Green),
            }),
            renderGaugeBar("Crescimento",
                std::clamp(c.gdpGrowthRate / 0.12 + 0.5, 0.0, 1.0),
                c.gdpGrowthRate >= 0 ? Color::Green : Color::Red),
            text("  " + fmtPct(c.gdpGrowthRate) +
                 (c.gdpGrowthRate >= 0 ? "  ▲ crescendo" : "  ▼ retraindo")) | dim,
            separator(),
            renderGaugeBar("Inflação",
                std::clamp(c.inflation / 0.25, 0.0, 1.0),
                c.inflation > 0.08 ? Color::Red : (c.inflation > 0.04 ? Color::Yellow : Color::Green)),
            text("  " + fmtPct(c.inflation) +
                 (c.inflation > 0.08 ? "  ⚠ alta" :
                  c.inflation > 0.04 ? "  moderada" : "  controlada")) | dim,
            renderGaugeBar("Desemprego",
                std::clamp(c.unemploymentRate / 0.30, 0.0, 1.0),
                c.unemploymentRate > 0.10 ? Color::Red : Color::Yellow),
            text("  " + fmtPct(c.unemploymentRate) +
                 (c.unemploymentRate > 0.15 ? "  ⚠ crítico" :
                  c.unemploymentRate > 0.08 ? "  elevado" : "  normal")) | dim,
            separator(),
            renderGaugeBar("Dívida/PIB",
                std::clamp(c.debtToGDP / 1.5, 0.0, 1.0),
                c.debtToGDP > 0.90 ? Color::Red : (c.debtToGDP > 0.60 ? Color::Yellow : Color::Green)),
            hbox({
                text("  Dívida: $" + fmtMoney(c.publicDebt.billions)),
                text("  Juros: " + fmtPct(c.interestRate)),
            }) | dim,
            hbox({
                text("  Reservas: $" + fmtMoney(c.foreignReserves.billions)),
                text("  Câmbio: " + fmtNum(c.exchangeRate)),
            }) | dim,
            text("  " + balLabel) | bold | color(balColor),
            separator(),
            renderGaugeBar("Desigualdade (Gini)",
                c.giniCoefficient,
                c.giniCoefficient > 0.50 ? Color::Red : Color::Yellow, 20),
        }) | border;

        // ── SEÇÃO SEGURANÇA & ORDEM ─────────────────────────────
        auto secSection = vbox({
            text(" SEGURANÇA & ORDEM PÚBLICA ") | bold | color(Color::Red) | hcenter,
            separator(),
            renderGaugeBar("Estabilidade",
                c.internalStability,
                c.internalStability < 0.40 ? Color::Red : Color::Blue),
            renderGaugeBar("Criminalidade",
                crimeIndex,
                crimeIndex > 0.60 ? Color::Red : (crimeIndex > 0.35 ? Color::Yellow : Color::Green)),
            text("  Estado de Direito: " + fmtPct(c.ruleOfLaw)) | dim,
            renderGaugeBar("Corrupção",
                c.corruption,
                c.corruption > 0.60 ? Color::Red : (c.corruption > 0.35 ? Color::Yellow : Color::GreenLight)),
            renderGaugeBar("Terrorismo",
                std::clamp(c.terrorismThreat / 0.50, 0.0, 1.0),
                c.terrorismThreat > 0.30 ? Color::Red : Color::Yellow),
            renderGaugeBar("Seg. Fronteira",
                c.borderSecurity, Color::Blue),
            renderGaugeBar("Defesa Cyber",
                c.cyberDefense, Color::Cyan),
            separator(),
            text("  Efetivo: " + fmtNum(c.military.activePersonnel) +
                 "  Reserva: " + fmtNum(c.military.reservePersonnel)) | dim,
            (c.military.nuclearWarheads > 0
                ? text("  ☢ Ogivas Nucleares: " + std::to_string(c.military.nuclearWarheads)) | color(Color::RedLight)
                : text("  Nuclear: Nenhuma") | dim),
        }) | border;

        // ── SEÇÃO DESENVOLVIMENTO HUMANO ────────────────────────
        Color hdiColor = c.hdi > 0.80 ? Color::Green :
                         c.hdi > 0.65 ? Color::Yellow : Color::Red;
        auto socialSection = vbox({
            text("╭─────── ♥ DESENVOLVIMENTO & SOCIEDADE ───────╮") | bold | color(Color::CyanLight),
            hbox({
                renderStatBox("IDH",          std::to_string(c.hdi).substr(0,5), hdiColor),
                renderStatBox("Expect. Vida", fmtNum(c.lifeExpectancy) + "a",   Color::Green),
            }),
            renderGaugeBar("Alfabetização",  c.literacyRate,       Color::Cyan),
            renderGaugeBar("Urbanização",    c.urbanizationRate,   Color::Blue),
            renderGaugeBar("Saúde Pública",  c.budgetHealthcare,    Color::Green, 18),
            renderGaugeBar("Educação",       c.budgetEducation,     Color::CyanLight, 18),
            renderGaugeBar("Previdência",    c.budgetSocialWelfare, Color::Magenta, 18),
            separator(),
            renderGaugeBar("Democracia",     c.democracy,          Color::Cyan),
            renderGaugeBar("Lib. Imprensa",  c.pressLiberty,       Color::CyanLight),
            separator(),
            text("  Pop: " + fmtPop(c.population) +
                 "  Cresc: " + fmtPct(c.populationGrowthRate)) | dim,
            text("  Nasc: " + fmtPct(c.birthRate) +
                 "  Óbito: " + fmtPct(c.deathRate) +
                 "  Id.Méd: " + fmtNum(c.medianAge) + "a") | dim,
        }) | border;

        // ── SEÇÃO ENERGIA & AMBIENTE ────────────────────────────
        auto envSection = vbox({
            text(" ☀ ENERGIA & MEIO AMBIENTE ") | bold | color(Color::GreenLight) | hcenter,
            separator(),
            renderGaugeBar("Renováveis",
                c.renewableShare,
                c.renewableShare > 0.50 ? Color::Green : Color::Yellow),
            text("  Produção: " + fmtNum(c.energyProductionTWh) + " TWh"
                 "  Consumo: "  + fmtNum(c.energyConsumptionTWh) + " TWh") | dim,
            [&]() -> Element {
                bool stateEnergy = (c.energyProductionTWh > c.energyConsumptionTWh);
                return text(stateEnergy
                    ? "  Balanço Energético: ▲ Exportador"
                    : "  Balanço Energético: ▼ Importador")
                    | color(stateEnergy ? Color::GreenLight : Color::RedLight);
            }(),
            separator(),
            text("  CO₂: " + fmtNum(c.co2EmissionsMT) + " MT"
                 "  Desmat: " + fmtPct(c.deforestationRate) + "/ano") | dim,
            renderGaugeBar("Qual. Ar",
                std::clamp(1.0 - c.airQualityIndex / 200.0, 0.0, 1.0),
                Color::Green, 14),
            text("  Temp Global Δ: +" +
                 std::to_string(engine_.getWorld().getGlobalTemperatureAnomaly()).substr(0,4) + "°C") | dim,
        }) | border;

        // ── EVENTOS ATIVOS ──────────────────────────────────────
        auto events = engine_.getEvents().getEventsForCountry(playerId_);
        Elements eventElements;
        eventElements.push_back(text(" ⚡ EVENTOS ATIVOS ⚡ ") | bold | color(Color::Magenta) | hcenter);
        eventElements.push_back(separator());
        bool anyActive = false;
        for (auto& ev : events) {
            if (!ev.active) continue;
            anyActive = true;
            Color evColor = ev.severity > 0.70 ? Color::RedLight :
                            ev.severity > 0.40 ? Color::Yellow : Color::GreenLight;
            int bars = (int)(ev.severity * 8);
            std::string sevBar = std::string(bars, '|') + std::string(8 - bars, '.');
            eventElements.push_back(hbox({
                text("  ⚡ ") | color(evColor),
                text(ev.title) | bold | color(evColor) | flex,
                text(" [" + sevBar + "] ") | color(evColor) | dim,
            }));
        }
        if (!anyActive) eventElements.push_back(text("  Nenhum evento ativo.") | dim);
        auto eventSection = vbox(eventElements) | border;

        // ── LAYOUT FINAL ────────────────────────────────────────
        return vbox({
            // Barra de resumo rápido
            [&]() -> Element {
                Color uc = c.unemploymentRate > 0.10 ? Color::RedLight :
                           c.unemploymentRate > 0.06 ? Color::Yellow : Color::GreenLight;
                return hbox({
                    text(" ▣ ") | color(Color::CyanLight) | bold,
                    text("PIB: ") | dim, text("$" + fmtMoney(c.gdp.billions)) | bold | color(Color::GreenLight),
                    text("  Cresc: ") | dim, text(fmtPct(c.gdpGrowthRate)) | bold
                        | color(c.gdpGrowthRate >= 0 ? Color::GreenLight : Color::RedLight),
                    text("  Infl: ") | dim, text(fmtPct(c.inflation)) | bold
                        | color(c.inflation > 0.06 ? Color::RedLight : Color::Yellow),
                    text("  Desemp: ") | dim, text(fmtPct(c.unemploymentRate)) | bold | color(uc),
                    text("  Juros: ") | dim, text(fmtPct(c.interestRate)) | bold | color(Color::Yellow),
                    text("  Dív/PIB: ") | dim, text(fmtPct(c.debtToGDP)) | bold
                        | color(c.debtToGDP > 0.80 ? Color::RedLight : Color::Yellow),
                    filler(),
                }) | bgcolor(Color::GrayDark);
            }(),
            hbox({
                // coluna esquerda: economia + segurança
                vbox({
                    econSection,
                    secSection,
                }) | flex,
                // coluna direita: aprovação + desenvolvimento + ambiente + eventos
                vbox({
                    approvalSection,
                    socialSection,
                    hbox({
                        envSection | flex,
                        eventSection | flex,
                    }),
                }) | flex,
            }),
        });
    });
}

// ============================================================
// ECONOMY TAB — sidebar navigation por categoria
// ============================================================

Component GameUI::createEconomyTab() {
    // ── Lambda: aplica todos os sliders de orçamento de uma vez ───────────
    auto applyBudget = [&] {
        BudgetAllocation ba;
        ba.defense        = budDefense_     / 100.0;
        ba.education      = budEducation_   / 100.0;
        ba.healthcare     = budHealthcare_  / 100.0;
        ba.infrastructure = budInfra_       / 100.0;
        ba.socialWelfare  = budSocial_      / 100.0;
        ba.science        = budScience_     / 100.0;
        ba.environment    = budEnv_         / 100.0;
        ba.administration = budAdmin_       / 100.0;
        ba.debtService    = budDebt_        / 100.0;
        ba.security       = budSecurity_    / 100.0;
        ba.agriculture    = budAgriculture_ / 100.0;
        ba.culture        = budCulture_     / 100.0;
        ba.transport      = budTransport_   / 100.0;
        ba.totalGdpRatio  = spendingRatio_  / 100.0;
        ba.normalize();
        engine_.getEconomy().setBudgetAllocation(playerId_, ba);
        addNotification("✓ Orçamento aplicado e normalizado.");
    };

    // ── PAINEL 0: Panorama Geral ──────────────────────────────────────────
    auto btnPrint = Button("  Imprimir $50B     ", [&] {
        engine_.getEconomy().printMoney(playerId_, 50.0);
        addNotification("⚠ $50B injetados! Inflação aumentará.");
    });
    auto btnDebt = Button("  Emitir Dívida $50B", [&] {
        engine_.getEconomy().issueDebt(playerId_, 50.0);
        addNotification("✓ $50B de dívida emitida.");
    });
    auto btnPayDebt = Button("  Pagar Dívida $50B ", [&] {
        auto& c = engine_.getWorld().getCountry(playerId_);
        double avail = std::min(50.0, c.foreignReserves.billions);
        if (avail <= 0.0) {
            addNotification("✗ Sem reservas para pagar dívida.");
        } else {
            engine_.getEconomy().payDebt(playerId_, 50.0);
            addNotification("✓ $" + fmtMoney(std::min(50.0, avail)) + " de dívida pagos com reservas.");
        }
    });
    auto panoramaPanel = Container::Vertical({btnPrint, btnDebt, btnPayDebt});

    // ── PAINEL 1: Impostos ────────────────────────────────────────────────
    auto sldTaxLow    = Slider("IR Baixo (0-50%):     ", &taxLow_,    0, 50, 1);
    auto sldTaxMid    = Slider("IR Médio (0-60%):     ", &taxMid_,    0, 60, 1);
    auto sldTaxHigh   = Slider("IR Alto (0-70%):      ", &taxHigh_,   0, 70, 1);
    auto sldTaxCorp   = Slider("Corporativo (0-60%):  ", &taxCorp_,   0, 60, 1);
    auto sldTaxSales  = Slider("IVA/Consumo (0-40%):  ", &taxSales_,  0, 40, 1);
    auto sldTaxImport = Slider("Tarifa Import (0-50%):", &taxImport_, 0, 50, 1);
    auto btnApplyTax  = Button(" ✓ Aplicar Impostos ", [&] {
        TaxPolicy tp;
        tp.incomeTaxLow  = taxLow_    / 100.0;
        tp.incomeTaxMid  = taxMid_    / 100.0;
        tp.incomeTaxHigh = taxHigh_   / 100.0;
        tp.corporateTax  = taxCorp_   / 100.0;
        tp.salesTax      = taxSales_  / 100.0;
        tp.importTariff  = taxImport_ / 100.0;
        engine_.getEconomy().setTaxPolicy(playerId_, tp);
        auto& coun = engine_.getWorld().getCountry(playerId_);
        coun.incomeTaxLow  = tp.incomeTaxLow;
        coun.incomeTaxMid  = tp.incomeTaxMid;
        coun.incomeTaxHigh = tp.incomeTaxHigh;
        coun.corporateTax  = tp.corporateTax;
        coun.salesTax      = tp.salesTax;
        addNotification("✓ Política fiscal atualizada.");
    });
    auto taxPanel = Container::Vertical({sldTaxLow, sldTaxMid, sldTaxHigh,
                                         sldTaxCorp, sldTaxSales, sldTaxImport, btnApplyTax});

    // ── PAINEL 2: Gastos Gov. (% PIB) ────────────────────────────────────
    auto sldSpending    = Slider("Gastos Gov. (% PIB):", &spendingRatio_, 10, 55, 1);
    auto btnApplySpend  = Button(" ✓ Aplicar Nível de Gastos ", [&, applyBudget] {
        engine_.getEconomy().setSpendingRatio(playerId_, spendingRatio_ / 100.0);
        applyBudget();
        std::string msg = spendingRatio_ >= 30 ? "(expansionista)" :
                          spendingRatio_ <= 18 ? "(austeridade)" : "(moderado)";
        addNotification("✓ Gastos: " + std::to_string(spendingRatio_) + "% PIB " + msg);
    });
    auto spendingPanel = Container::Vertical({sldSpending, btnApplySpend});

    // ── PAINEIS 3-15: Um por categoria de orçamento ────────────────────────
    //  Cada painel tem: slider da categoria + botão aplicar orçamento total
    auto sldDefense   = Slider("% do orçamento: ", &budDefense_,     0, 50, 1);
    auto sldEdu       = Slider("% do orçamento: ", &budEducation_,   0, 40, 1);
    auto sldHealth    = Slider("% do orçamento: ", &budHealthcare_,  0, 40, 1);
    auto sldSocial    = Slider("% do orçamento: ", &budSocial_,      0, 50, 1);
    auto sldScience   = Slider("% do orçamento: ", &budScience_,     0, 20, 1);
    auto sldInfra     = Slider("% do orçamento: ", &budInfra_,       0, 30, 1);
    auto sldTransport = Slider("% do orçamento: ", &budTransport_,   0, 20, 1);
    auto sldEnv       = Slider("% do orçamento: ", &budEnv_,         0, 20, 1);
    auto sldSecurity  = Slider("% do orçamento: ", &budSecurity_,    0, 20, 1);
    auto sldAgri      = Slider("% do orçamento: ", &budAgriculture_, 0, 20, 1);
    auto sldCulture   = Slider("% do orçamento: ", &budCulture_,     0, 10, 1);
    auto sldAdmin     = Slider("% do orçamento: ", &budAdmin_,       0, 20, 1);
    auto sldDebt      = Slider("% do orçamento: ", &budDebt_,        0, 30, 1);

    // Botões de aplicar (um por painel para FTXUI não duplicar componente)
    auto mkBtn = [&, applyBudget](const std::string& sec) {
        return Button(" ✓ Aplicar (" + sec + ") ", [&, applyBudget] { applyBudget(); });
    };
    auto bDef  = mkBtn("Defesa");
    auto bEdu  = mkBtn("Educação");
    auto bHea  = mkBtn("Saúde");
    auto bSoc  = mkBtn("Previdência");
    auto bSci  = mkBtn("Ciência");
    auto bInf  = mkBtn("Infraestr.");
    auto bTra  = mkBtn("Transporte");
    auto bEnv  = mkBtn("Amb./Energ.");
    auto bSec  = mkBtn("Seg.Pública");
    auto bAgr  = mkBtn("Agricultura");
    auto bCul  = mkBtn("Cult./Mídia");
    auto bAdm  = mkBtn("Administr.");
    auto bDbt  = mkBtn("Serv.Dívida");

    auto makePanel3 = [](Component sld, Component btn) {
        return Container::Vertical({sld, btn});
    };

    auto defensePanel   = makePanel3(sldDefense,   bDef);
    auto eduPanel       = makePanel3(sldEdu,        bEdu);
    auto healthPanel    = makePanel3(sldHealth,     bHea);
    auto socialPanel    = makePanel3(sldSocial,     bSoc);
    auto sciencePanel   = makePanel3(sldScience,    bSci);
    auto infraPanel     = Container::Vertical({sldInfra,     bInf});
    auto transportPanel = makePanel3(sldTransport,  bTra);
    auto envPanel       = makePanel3(sldEnv,        bEnv);
    auto securityPanel  = makePanel3(sldSecurity,   bSec);
    auto agriPanel      = makePanel3(sldAgri,       bAgr);
    auto culturePanel   = makePanel3(sldCulture,    bCul);
    auto adminPanel     = makePanel3(sldAdmin,      bAdm);
    auto debtPanel      = makePanel3(sldDebt,       bDbt);

    // ── Menu lateral e container de sub-abas ─────────────────────────────
    auto sideMenu    = Menu(&ecoSubTabNames_, &ecoMode_);
    auto tabContent  = Container::Tab({
        panoramaPanel,  // 0
        taxPanel,       // 1
        spendingPanel,  // 2
        defensePanel,   // 3
        eduPanel,       // 4
        healthPanel,    // 5
        socialPanel,    // 6
        sciencePanel,   // 7
        infraPanel,     // 8
        transportPanel, // 9
        envPanel,       // 10
        securityPanel,  // 11
        agriPanel,      // 12
        culturePanel,   // 13
        adminPanel,     // 14
        debtPanel,      // 15
    }, &ecoMode_);

    auto mainContainer = Container::Horizontal({sideMenu, tabContent});

    return Renderer(mainContainer, [&,
        sideMenu, tabContent,
        panoramaPanel, taxPanel, spendingPanel,
        defensePanel, eduPanel, healthPanel, socialPanel, sciencePanel,
        infraPanel, transportPanel, envPanel, securityPanel,
        agriPanel, culturePanel, adminPanel, debtPanel,
        sldInfra, bInf] {
        auto& c   = engine_.getWorld().getCountry(playerId_);
        auto mkt  = engine_.getEconomy().getMarketIndicators(playerId_);
        double totalBudget = c.gdp.billions * (spendingRatio_ / 100.0);

        // ── Totalizador dos sliders (não-normalizado, para feedback) ───────
        int rawTotal = budDefense_ + budEducation_ + budHealthcare_ + budInfra_ +
                       budSocial_ + budScience_ + budEnv_ + budAdmin_ + budDebt_ +
                       budSecurity_ + budAgriculture_ + budCulture_ + budTransport_;
        Color totColor = (rawTotal > 120) ? Color::RedLight :
                         (rawTotal > 100) ? Color::Yellow : Color::GreenLight;

        // ── Header de estatísticas macro (sempre visível) ─────────────────
        double fb  = c.governmentBudget.billions;
        Color balC = fb >= 0 ? Color::GreenLight : Color::RedLight;
        std::string balLabel = fb >= 0
            ? "SUPERÁVIT +$" + fmtMoney(fb)
            : "DÉFICIT  -$"  + fmtMoney(-fb);

        auto statsHeader = vbox({
            text(" ₿ GESTÃO ECONÔMICA ─ " + c.name + " ") | bold | hcenter | color(Color::GreenLight),
            separator(),
            hbox({
                renderStatBox("PIB",     "$" + fmtMoney(c.gdp.billions),   Color::GreenLight),
                renderStatBox("Cresc.",  fmtPct(c.gdpGrowthRate),
                             c.gdpGrowthRate >= 0 ? Color::GreenLight : Color::RedLight),
                renderStatBox("Inflação",fmtPct(c.inflation),
                             c.inflation > 0.05 ? Color::RedLight : Color::Yellow),
                renderStatBox("Desempr.",fmtPct(c.unemploymentRate),
                             c.unemploymentRate > 0.08 ? Color::RedLight : Color::Yellow),
                renderStatBox("Dív/PIB", fmtPct(c.debtToGDP),
                             c.debtToGDP > 0.80 ? Color::RedLight : Color::Yellow),
            }),
            hbox({
                renderStatBox("Receita", "$" + fmtMoney(c.governmentRevenue.billions), Color::GreenLight),
                renderStatBox("Gastos",  "$" + fmtMoney(c.governmentExpenditure.billions),
                             c.governmentExpenditure.billions > c.governmentRevenue.billions
                                 ? Color::RedLight : Color::Yellow),
                renderStatBox("Mercado", std::to_string((int)mkt.stockMarketIndex) + "pts", Color::CyanLight),
                renderStatBox("Rating",  std::to_string(mkt.creditRating).substr(0,4), Color::Cyan),
                text("  " + balLabel) | bold | color(balC) | vcenter | flex,
            }),
            // Banco Central info
            hbox({
                text("  🏦 Banco Central: Juros = ") | dim,
                text(fmtPct(c.interestRate)) | bold | color(Color::Yellow),
                text(" (ajustado a cada 3 meses)") | dim,
                text("   Câmbio: " + fmtNum(c.exchangeRate)) | dim,
                text("   Reservas: $" + fmtMoney(c.foreignReserves.billions)) | dim,
            }),
        });

        // ── Conteúdo contextual por aba ──────────────────────────────────
        Element rightContent = text("");

        auto fmtBudSlice = [&](double frac) -> std::string {
            return "$" + fmtMoney(totalBudget * frac) + " (" + fmtPct(frac) + " orç.)";
        };

        switch (ecoMode_) {
        case 0: { // Panorama
            auto ba = engine_.getEconomy().getBudgetAllocation(playerId_);
            rightContent = vbox({
                text(" ▣ DISTRIBUIÇÃO DO ORÇAMENTO ATUAL ") | bold | color(Color::Yellow) | hcenter,
                separator(),
                renderGaugeBar("Defesa/Mil.",    ba.defense,        Color::Red,     18),
                renderGaugeBar("Educação",       ba.education,      Color::Cyan,    18),
                renderGaugeBar("Saúde",          ba.healthcare,     Color::Green,   18),
                renderGaugeBar("Previdência",    ba.socialWelfare,  Color::Magenta, 18),
                renderGaugeBar("Ciência/Tec.",   ba.science,        Color::Blue,    18),
                renderGaugeBar("Infraestr.",     ba.infrastructure, Color::Yellow,  18),
                renderGaugeBar("Transporte",     ba.transport,      Color::Yellow,  18),
                renderGaugeBar("Amb./Energia",   ba.environment,    Color::GreenLight, 18),
                renderGaugeBar("Seg.Pública",    ba.security,       Color::RedLight,18),
                renderGaugeBar("Agricultura",    ba.agriculture,    Color::GreenLight, 18),
                renderGaugeBar("Mídia/Cultura",  ba.culture,        Color::Magenta, 18),
                renderGaugeBar("Administração",  ba.administration, Color::GrayLight, 18),
                renderGaugeBar("Serv.Dívida",    ba.debtService,    Color::RedLight, 18),
                separator(),
                text("  Total orçamento est.: $" + fmtMoney(totalBudget) + "/ano") | dim,
                separator(),
                text(" AÇÕES FISCAIS ") | bold | color(Color::Yellow),
                panoramaPanel->Render(),
            });
            break;
        }
        case 1: { // Impostos
            rightContent = vbox({
                text(" POLÍTICA TRIBUTÁRIA ") | bold | color(Color::Yellow) | hcenter,
                separator(),
                hbox({
                    text("  IR Baixo:   " + std::to_string(taxLow_)    + "%") | size(WIDTH, EQUAL, 22),
                    text("  IR Médio:   " + std::to_string(taxMid_)    + "%") | size(WIDTH, EQUAL, 22),
                    text("  IR Alto:    " + std::to_string(taxHigh_)   + "%"),
                }) | dim,
                hbox({
                    text("  Corporativo:" + std::to_string(taxCorp_)   + "%") | size(WIDTH, EQUAL, 22),
                    text("  IVA/Consum.:" + std::to_string(taxSales_)  + "%") | size(WIDTH, EQUAL, 22),
                    text("  Importação: " + std::to_string(taxImport_) + "%"),
                }) | dim,
                separator(),
                text("  Receita atual: $" + fmtMoney(c.governmentRevenue.billions) + "/ano") | dim,
                text("  Carga tributária: " + fmtPct(c.taxBurden) + " do PIB") | dim,
                separator(),
                taxPanel->Render(),
            });
            break;
        }
        case 2: { // Gastos Gov.
            rightContent = vbox({
                text(" NÍVEL DE GASTOS GOVERNAMENTAIS ") | bold | color(Color::Yellow) | hcenter,
                separator(),
                text("  Gastos atuais como % do PIB: " + std::to_string(spendingRatio_) + "%") | dim,
                text("  Estimativa: $" + fmtMoney(totalBudget) + "/ano") | dim,
                text("  Receita: $" + fmtMoney(c.governmentRevenue.billions) + "/ano") | dim,
                [&]() -> Element {
                    double diff = c.governmentRevenue.billions - totalBudget;
                    if (diff >= 0) return text("  ▲ Superávit estimado: +$" + fmtMoney(diff)) | color(Color::GreenLight);
                    else           return text("  ▼ Déficit estimado:   -$" + fmtMoney(-diff)) | color(Color::RedLight);
                }(),
                separator(),
                text("  Referência: Mundo desenvolvido ~ 35–50% | Emergente ~ 20–30%") | dim,
                spendingPanel->Render(),
            });
            break;
        }
        case 3: { // Defesa
            rightContent = vbox({
                text(" DEFESA / FORÇAS MILITARES ") | bold | color(Color::Red) | hcenter,
                separator(),
                text("  Alocação atual:  " + std::to_string(budDefense_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budDefense_ / 100.0)) | dim,
                separator(),
                text("  Efetivo ativo:    " + fmtNum(c.military.activePersonnel)) | dim,
                text("  Reservistas:      " + fmtNum(c.military.reservePersonnel)) | dim,
                renderGaugeBar("Treinamento",   c.military.training,   Color::Yellow, 16),
                renderGaugeBar("Moral",         c.military.morale,     Color::Green,  16),
                renderGaugeBar("Tecnologia",    c.military.technology, Color::Cyan,   16),
                renderGaugeBar("Prontidão",     c.military.readiness,  Color::Blue,   16),
                (c.military.nuclearWarheads > 0
                    ? text("  ☢ Ogivas: " + std::to_string(c.military.nuclearWarheads)) | color(Color::RedLight)
                    : text("  Nuclear: Nenhuma") | dim),
                separator(),
                defensePanel->Render(),
            });
            break;
        }
        case 4: { // Educação
            rightContent = vbox({
                text(" EDUCAÇÃO E FORMAÇÃO ") | bold | color(Color::Cyan) | hcenter,
                separator(),
                text("  Alocação atual:   " + std::to_string(budEducation_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budEducation_ / 100.0)) | dim,
                separator(),
                renderGaugeBar("Alfabetização",  c.literacyRate,       Color::Cyan,  18),
                text("  IDH:           " + std::to_string(c.hdi).substr(0, 5)) | dim,
                text("  Gasto médio ref.: Países OCDE ~5% PIB | Meta ONU ~4% PIB") | dim,
                separator(),
                eduPanel->Render(),
            });
            break;
        }
        case 5: { // Saúde
            rightContent = vbox({
                text(" SAÚDE PÚBLICA ") | bold | color(Color::Green) | hcenter,
                separator(),
                text("  Alocação atual:   " + std::to_string(budHealthcare_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budHealthcare_ / 100.0)) | dim,
                separator(),
                text("  Expect. de vida:  " + fmtNum(c.lifeExpectancy) + " anos") | dim,
                text("  Urbanização:      " + fmtPct(c.urbanizationRate)) | dim,
                text("  Nasc./mil:        " + fmtPct(c.birthRate)) | dim,
                text("  Óbito/mil:        " + fmtPct(c.deathRate)) | dim,
                separator(),
                healthPanel->Render(),
            });
            break;
        }
        case 6: { // Previdência Social
            rightContent = vbox({
                text(" PREVIDÊNCIA E PROTEÇÃO SOCIAL ") | bold | color(Color::Magenta) | hcenter,
                separator(),
                text("  Alocação atual:   " + std::to_string(budSocial_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budSocial_ / 100.0)) | dim,
                separator(),
                renderGaugeBar("Desigualdade (Gini)", c.giniCoefficient,
                    c.giniCoefficient > 0.50 ? Color::Red : Color::Yellow, 22),
                text("  Desemprego:       " + fmtPct(c.unemploymentRate)) | dim,
                text("  Id. mediana:      " + fmtNum(c.medianAge) + " anos") | dim,
                text("  Ref.: OCDE gasta ~40% do orçamento em proteção social") | dim,
                separator(),
                socialPanel->Render(),
            });
            break;
        }
        case 7: { // Ciência/Tec.
            rightContent = vbox({
                text(" CIÊNCIA, TECNOLOGIA E INOVAÇÃO ") | bold | color(Color::Blue) | hcenter,
                separator(),
                text("  Alocação atual:   " + std::to_string(budScience_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budScience_ / 100.0)) | dim,
                separator(),
                text("  IDH:              " + std::to_string(c.hdi).substr(0, 5)) | dim,
                renderGaugeBar("Cyber Defesa",  c.cyberDefense,          Color::Cyan,  18),
                text("  Potência militar tec.: " + std::to_string(c.military.technology).substr(0,4)) | dim,
                separator(),
                sciencePanel->Render(),
            });
            break;
        }
        case 8: { // Infraestrutura
            rightContent = vbox({
                text(" INFRAESTRUTURA GERAL ") | bold | color(Color::Yellow) | hcenter,
                separator(),
                text("  Alocação Infraestr.: " + std::to_string(budInfra_) + "% do orçamento") | dim,
                text("  Estimativa anual:    " + fmtBudSlice(budInfra_ / 100.0)) | dim,
                separator(),
                text("  Energia produção:    " + fmtNum(c.energyProductionTWh) + " TWh") | dim,
                text("  Energia consumo:     " + fmtNum(c.energyConsumptionTWh) + " TWh") | dim,
                text("  Coastline:           " + fmtNum(c.coastlineKm) + " km") | dim,
                text("  Área total:          " + fmtNum(c.areaSqKm) + " km²") | dim,
                separator(),
                text("  Slider Infraestrutura:"),
                sldInfra->Render(),
                bInf->Render(),
            });
            break;
        }
        case 9: { // Transporte
            rightContent = vbox({
                text(" TRANSPORTE PÚBLICO ") | bold | color(Color::Yellow) | hcenter,
                separator(),
                text("  Alocação atual:   " + std::to_string(budTransport_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budTransport_ / 100.0)) | dim,
                separator(),
                text("  Urbanização:      " + fmtPct(c.urbanizationRate)) | dim,
                text("  Qualidade infra impacta PIB, mobilidade e poluição") | dim,
                separator(),
                transportPanel->Render(),
            });
            break;
        }
        case 10: { // Ambiente/Energia
            rightContent = vbox({
                text(" MEIO AMBIENTE E ENERGIA ") | bold | color(Color::GreenLight) | hcenter,
                separator(),
                text("  Alocação atual:   " + std::to_string(budEnv_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budEnv_ / 100.0)) | dim,
                separator(),
                renderGaugeBar("Renováveis",       c.renewableShare,
                    c.renewableShare > 0.50 ? Color::Green : Color::Yellow, 18),
                text("  CO₂ emissões:     " + fmtNum(c.co2EmissionsMT) + " MT/ano") | dim,
                text("  Desmatamento:     " + fmtPct(c.deforestationRate) + "/ano") | dim,
                renderGaugeBar("Qualidade do Ar",
                    std::clamp(1.0 - c.airQualityIndex / 200.0, 0.0, 1.0),
                    Color::Green, 18),
                text("  Temperatura global Δ: +" +
                     std::to_string(engine_.getWorld().getGlobalTemperatureAnomaly()).substr(0,4) + "°C") | dim,
                separator(),
                envPanel->Render(),
            });
            break;
        }
        case 11: { // Seg. Pública
            double crimeIdx = std::clamp((1.0-c.ruleOfLaw)*0.45 + c.corruption*0.35 + c.terrorismThreat*0.20, 0.0, 1.0);
            rightContent = vbox({
                text(" SEGURANÇA PÚBLICA / CRIMINAL / IMIGRAÇÃO ") | bold | color(Color::Red) | hcenter,
                separator(),
                text("  Alocação atual:   " + std::to_string(budSecurity_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budSecurity_ / 100.0)) | dim,
                separator(),
                renderGaugeBar("Criminalidade",
                    crimeIdx, crimeIdx > 0.60 ? Color::Red : Color::Yellow, 18),
                renderGaugeBar("Estado de Direito", c.ruleOfLaw,        Color::Green,  18),
                renderGaugeBar("Corrupção",         c.corruption,
                    c.corruption > 0.60 ? Color::Red : Color::Yellow, 18),
                renderGaugeBar("Terrorismo",
                    std::clamp(c.terrorismThreat / 0.50, 0.0, 1.0), Color::RedLight, 18),
                renderGaugeBar("Seg. Fronteira",    c.borderSecurity,   Color::Blue,   18),
                separator(),
                securityPanel->Render(),
            });
            break;
        }
        case 12: { // Agricultura
            rightContent = vbox({
                text(" AGRICULTURA E DESENVOLVIMENTO RURAL ") | bold | color(Color::GreenLight) | hcenter,
                separator(),
                text("  Alocação atual:   " + std::to_string(budAgriculture_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budAgriculture_ / 100.0)) | dim,
                separator(),
                text("  Pop. rural:       " + fmtPct(1.0 - c.urbanizationRate)) | dim,
                text("  Inverso urbaniz.: " + fmtPct(1.0 - c.urbanizationRate)) | dim,
                text("  Investimento beneficia segurança alimentar, Gini e exportações") | dim,
                separator(),
                agriPanel->Render(),
            });
            break;
        }
        case 13: { // Mídia/Cultura
            rightContent = vbox({
                text(" MÍDIA, CULTURA E ENTRETENIMENTO ") | bold | color(Color::Magenta) | hcenter,
                separator(),
                text("  Alocação atual:   " + std::to_string(budCulture_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budCulture_ / 100.0)) | dim,
                separator(),
                renderGaugeBar("Lib. Imprensa",  c.pressLiberty,  Color::Cyan,   18),
                text("  Soft Power:       " + fmtPct(c.softPower)) | dim,
                text("  Investimento em cultura aumenta soft power e aprovação") | dim,
                separator(),
                culturePanel->Render(),
            });
            break;
        }
        case 14: { // Administração
            rightContent = vbox({
                text(" ADMINISTRAÇÃO PÚBLICA ") | bold | color(Color::GrayLight) | hcenter,
                separator(),
                text("  Alocação atual:   " + std::to_string(budAdmin_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budAdmin_ / 100.0)) | dim,
                separator(),
                renderGaugeBar("Corrupção",        c.corruption,
                    c.corruption > 0.50 ? Color::Red : Color::Yellow, 18),
                renderGaugeBar("Confiança Instit.", c.institutionalTrust, Color::Cyan, 18),
                renderGaugeBar("Democracia",       c.democracy,       Color::Blue,  18),
                text("  Efic. fiscal:     " + fmtPct(c.ruleOfLaw)) | dim,
                separator(),
                adminPanel->Render(),
            });
            break;
        }
        case 15: { // Serviço da Dívida
            rightContent = vbox({
                text(" SERVIÇO DA DÍVIDA PÚBLICA ") | bold | color(Color::RedLight) | hcenter,
                separator(),
                text("  Alocação atual:   " + std::to_string(budDebt_) + "% do orçamento") | dim,
                text("  Estimativa anual: " + fmtBudSlice(budDebt_ / 100.0)) | dim,
                separator(),
                text("  Dívida pública:   $" + fmtMoney(c.publicDebt.billions)) | dim,
                renderGaugeBar("Dívida/PIB",
                    std::clamp(c.debtToGDP / 1.5, 0.0, 1.0),
                    c.debtToGDP > 0.90 ? Color::Red : Color::Yellow, 18),
                text("  Taxa de juros BC: " + fmtPct(c.interestRate)) | dim,
                text("  Reservas externas:$" + fmtMoney(c.foreignReserves.billions)) | dim,
                text("  Reduzir dívida melhora rating de crédito e custo de capital") | dim,
                separator(),
                debtPanel->Render(),
            });
            break;
        }
        default:
            rightContent = text(" Selecione uma categoria ") | dim | hcenter;
        }

        // ── Totalizador flutuante dos sliders ────────────────────────────
        auto totalizer = hbox({
            text("  Total sliders: " + std::to_string(rawTotal) + "% ") | color(totColor),
            text("(normalizado ao aplicar) ") | dim,
        });

        return vbox({
            statsHeader,
            totalizer,
            separator(),
            hbox({
                sideMenu->Render() | border | size(WIDTH, EQUAL, 18),
                rightContent | border | flex,
            }) | flex,
        });
    });
}


// ============================================================
// DIPLOMACY TAB
// ============================================================

Component GameUI::createDiplomacyTab() {
    diploCountryNames_.clear();
    diploOtherIds_.clear();
    auto countryIds = engine_.getWorld().getAllCountryIds();
    for (auto cid : countryIds) {
        if (cid == playerId_) continue;
        diploCountryNames_.push_back(engine_.getWorld().getCountry(cid).name);
        diploOtherIds_.push_back(cid);
    }

    // Action buttons
    auto btnTrade = Button("Acordo Comercial", [&] {
        if (diploSelected_ < 0 || diploSelected_ >= (int)diploOtherIds_.size()) return;
        DiplomaticProposal prop;
        prop.proposer = playerId_;
        prop.target = diploOtherIds_[diploSelected_];
        prop.type = DiplomaticProposal::Type::TRADE_AGREEMENT;
        prop.proposedDate = engine_.getCurrentDate();
        prop.durationYears = 5;
        prop.description = "Acordo Comercial";
        engine_.getDiplomacy().sendProposal(prop);
        addNotification("✓ Proposta de acordo comercial enviada para " +
                       engine_.getWorld().getCountry(diploOtherIds_[diploSelected_]).name);
    });

    auto btnAlliance = Button("Aliança Militar", [&] {
        if (diploSelected_ < 0 || diploSelected_ >= (int)diploOtherIds_.size()) return;
        DiplomaticProposal prop;
        prop.proposer = playerId_;
        prop.target = diploOtherIds_[diploSelected_];
        prop.type = DiplomaticProposal::Type::MILITARY_ALLIANCE;
        prop.proposedDate = engine_.getCurrentDate();
        prop.durationYears = 10;
        prop.description = "Aliança Militar";
        engine_.getDiplomacy().sendProposal(prop);
        addNotification("✓ Proposta de aliança militar enviada.");
    });

    auto btnSanctions = Button("Impor Sanções", [&] {
        if (diploSelected_ < 0 || diploSelected_ >= (int)diploOtherIds_.size()) return;
        engine_.getDiplomacy().declareSanctions(playerId_, diploOtherIds_[diploSelected_]);
        addNotification("✓ Sanções impostas sobre " +
                       engine_.getWorld().getCountry(diploOtherIds_[diploSelected_]).name);
    });

    auto btnWar = Button("⚔ Declarar Guerra", [&] {
        if (diploSelected_ < 0 || diploSelected_ >= (int)diploOtherIds_.size()) return;
        engine_.getDiplomacy().declareWar(playerId_, diploOtherIds_[diploSelected_], "Razões de Estado");
        addNotification("⚔ GUERRA declarada contra " +
                       engine_.getWorld().getCountry(diploOtherIds_[diploSelected_]).name + "!");
    });

    auto btnPeace = Button("🕊 Propor Paz", [&] {
        if (diploSelected_ < 0 || diploSelected_ >= (int)diploOtherIds_.size()) return;
        engine_.getDiplomacy().proposePeace(playerId_, diploOtherIds_[diploSelected_]);
        addNotification("🕊 Proposta de paz enviada.");
    });

    auto btnAid = Button("Enviar Ajuda $10B", [&] {
        if (diploSelected_ < 0 || diploSelected_ >= (int)diploOtherIds_.size()) return;
        engine_.getDiplomacy().sendForeignAid(playerId_, diploOtherIds_[diploSelected_], Money(10.0));
        addNotification("✓ $10B de ajuda enviados.");
    });

    auto menu = Menu(&diploCountryNames_, &diploSelected_);
    auto buttons = Container::Vertical({
        btnTrade, btnAlliance, btnSanctions, btnWar, btnPeace, btnAid,
    });

    auto container = Container::Horizontal({menu, buttons});

    return Renderer(container, [&, container] {
        // Build relations table
        Elements tableRows;
        tableRows.push_back(
            hbox({
                text(" País") | bold | size(WIDTH, EQUAL, 22),
                text("♥ Rel.") | bold | color(Color::Yellow) | size(WIDTH, EQUAL, 8),
                text("Status     ") | bold | size(WIDTH, EQUAL, 12),
                text("Acordos") | bold | flex,
            }) | bgcolor(Color::GrayDark)
        );

        for (size_t i = 0; i < diploOtherIds_.size(); ++i) {
            auto cid = diploOtherIds_[i];
            auto& other = engine_.getWorld().getCountry(cid);
            auto& rel = engine_.getWorld().getRelation(playerId_, cid);

            std::string statusStr;
            Color statusColor;
            switch (rel.status) {
                case DiplomaticStatus::ALLIED:    statusStr = "★ ALIADO";   statusColor = Color::GreenLight; break;
                case DiplomaticStatus::FRIENDLY:  statusStr = "▲ AMIGÁVEL"; statusColor = Color::Green; break;
                case DiplomaticStatus::NEUTRAL:   statusStr = "□ NEUTRO";   statusColor = Color::Yellow; break;
                case DiplomaticStatus::COOL:      statusStr = "▽ FRIO";     statusColor = Color::GrayLight; break;
                case DiplomaticStatus::HOSTILE:   statusStr = "▼ HOSTIL";   statusColor = Color::Red; break;
                case DiplomaticStatus::AT_WAR:    statusStr = "⚔ GUERRA!";  statusColor = Color::RedLight; break;
                case DiplomaticStatus::EMBARGO:   statusStr = "⛔ EMBARGO";  statusColor = Color::Magenta; break;
                case DiplomaticStatus::SANCTIONS: statusStr = "⚠ SANÇÕES";  statusColor = Color::MagentaLight; break;
                default: statusStr = "???"; statusColor = Color::White; break;
            }

            std::string acordos;
            if (rel.hasTradeAgreement)   acordos += "[Comrc] ";
            if (rel.hasMilitaryAlliance) acordos += "[Aliân] ";
            if (rel.hasSanctions)        acordos += "[Sanção] ";
            if (acordos.empty()) acordos = "─ nenhum";

            Color relColor = rel.relations > 50 ? Color::GreenLight :
                            rel.relations > 0 ? Color::Yellow : Color::RedLight;
            bool selected = ((int)i == diploSelected_);
            // Mini barra de relações
            int relBars = (int)((rel.relations + 100.0) / 200.0 * 5);
            relBars = std::clamp(relBars, 0, 5);
            std::string relBar = std::string(relBars, '|') + std::string(5 - relBars, '.');

            auto row = hbox({
                text((selected ? "▸ " : "  ") + other.name) | size(WIDTH, EQUAL, 22)
                    | (selected ? bold : nothing),
                text(relBar + " " + std::to_string((int)rel.relations)) | color(relColor) | size(WIDTH, EQUAL, 8),
                text(statusStr) | color(statusColor) | size(WIDTH, EQUAL, 12),
                text(acordos) | dim | flex,
            });
            if (selected) tableRows.push_back(row | bgcolor(Color::GrayDark));
            else tableRows.push_back(row);
        }

        // Pending proposals
        auto pendentes = engine_.getDiplomacy().getPendingProposals(playerId_);
        Elements proposalElements;
        if (!pendentes.empty()) {
            proposalElements.push_back(
                text(" ⚡ PROPOSTAS PENDENTES: " + std::to_string(pendentes.size()) + " ") |
                bold | color(Color::Yellow) | hcenter
            );
            for (auto& p : pendentes) {
                auto& from = engine_.getWorld().getCountry(p.proposer);
                proposalElements.push_back(
                    hbox({
                        text("  ▸ " + p.description) | bold | color(Color::Cyan) | flex,
                        text(" de " + from.name) | dim,
                    })
                );
            }
        }

        return vbox({
            text(" ⚔ DIPLOMACIA & RELAÇÕES INTERNACIONAIS ") | bold | hcenter | color(Color::CyanLight),
            separator(),
            hbox({
                vbox(tableRows) | border | flex,
                vbox({
                    text(" AÇÕES ") | bold | color(Color::Yellow) | hcenter,
                    separator(),
                    container->Render(),
                    separator(),
                    proposalElements.empty()
                        ? (text("  Sem propostas pendentes.") | dim)
                        : vbox(proposalElements),
                }) | border | size(WIDTH, EQUAL, 32),
            }),
        });
    });
}

// ============================================================
// MILITARY TAB
// ============================================================

Component GameUI::createMilitaryTab() {
    auto btnCreateUnit = Button("Criar Unidade (5000)", [&] {
        engine_.getMilitary().createUnit(playerId_, MilitaryBranch::ARMY, "Nova Brigada", 5000);
        addNotification("✓ Unidade criada com 5000 efetivos.");
    });
    auto btnConscript = Button("Mobilizar 10000", [&] {
        engine_.getMilitary().conscript(playerId_, 10000);
        addNotification("✓ 10000 reservistas mobilizados.");
    });
    auto btnDemobilize = Button("Desmobilizar 10000", [&] {
        engine_.getMilitary().demobilize(playerId_, 10000);
        addNotification("✓ 10000 tropas desmobilizadas.");
    });
    auto btnNavy = Button("Criar Unid. Naval (2000)", [&] {
        engine_.getMilitary().createUnit(playerId_, MilitaryBranch::NAVY, "Esquadra Naval", 2000);
        addNotification("✓ Unidade naval criada.");
    });
    auto btnAir = Button("Criar Unid. Aérea (1000)", [&] {
        engine_.getMilitary().createUnit(playerId_, MilitaryBranch::AIR_FORCE, "Ala Aérea", 1000);
        addNotification("✓ Unidade aérea criada.");
    });
    auto btnSpecOps = Button("Criar Forças Esp. (500)", [&] {
        engine_.getMilitary().createUnit(playerId_, MilitaryBranch::SPECIAL_FORCES, "Ops Especiais", 500);
        addNotification("✓ Unidade de forças especiais criada.");
    });

    auto buttons = Container::Vertical({
        btnCreateUnit, btnNavy, btnAir, btnSpecOps,
        btnConscript, btnDemobilize,
    });

    return Renderer(buttons, [&, buttons] {
        auto& c = engine_.getWorld().getCountry(playerId_);
        auto forces = engine_.getMilitary().getForces(playerId_);
        auto conflicts = engine_.getMilitary().getActiveConflicts();
        auto units = engine_.getMilitary().getUnits(playerId_);
        double milPower = engine_.getMilitary().getMilitaryPower(playerId_);

        // Forces overview
        auto forcesSection = vbox({
            text(" ★ FORÇAS ARMADAS ") | bold | color(Color::RedLight) | hcenter,
            separator(),
            hbox({
                renderStatBox("Efetivo", fmtNum(forces.activePersonnel), Color::GreenLight),
                renderStatBox("Reserva", fmtNum(forces.reservePersonnel), Color::Yellow),
                renderStatBox("Nuclear", forces.nuclearWarheads > 0
                    ? ("\u2622 " + std::to_string(forces.nuclearWarheads)) : "Nenhuma", Color::RedLight),
                renderStatBox("Poder Mil.", std::to_string(milPower).substr(0, 5), Color::CyanLight),
            }),
            renderGaugeBar("Orçam. Defesa",   c.budgetDefense,         Color::Red),
            renderGaugeBar("Treinamento",     forces.training,          Color::Yellow, 14),
            renderGaugeBar("Moral",           forces.morale,            Color::Green, 14),
            renderGaugeBar("Prontidão",       forces.readiness,         Color::Cyan, 14),
        }) | border;

        // Units table
        Elements unitRows;
        unitRows.push_back(
            hbox({
                text("ID") | bold | size(WIDTH, EQUAL, 5),
                text("Nome") | bold | size(WIDTH, EQUAL, 22),
                text("Ramo") | bold | size(WIDTH, EQUAL, 14),
                text("Efetivo") | bold | size(WIDTH, EQUAL, 10),
                text("Força") | bold | size(WIDTH, EQUAL, 8),
            }) | bgcolor(Color::GrayDark)
        );
        for (auto& u : units) {
            std::string branchStr;
            switch (u.branch) {
                case MilitaryBranch::ARMY: branchStr = "Exército"; break;
                case MilitaryBranch::NAVY: branchStr = "Marinha"; break;
                case MilitaryBranch::AIR_FORCE: branchStr = "Força Aérea"; break;
                case MilitaryBranch::MARINES: branchStr = "Fuzileiros"; break;
                case MilitaryBranch::SPECIAL_FORCES: branchStr = "Forças Esp."; break;
                case MilitaryBranch::CYBER_COMMAND: branchStr = "Cyber"; break;
                default: branchStr = "Outro"; break;
            }
            unitRows.push_back(hbox({
                text(std::to_string(u.id)) | size(WIDTH, EQUAL, 5),
                text(u.name) | size(WIDTH, EQUAL, 20),
                text(branchStr) | size(WIDTH, EQUAL, 14),
                text(std::to_string(u.personnel)) | size(WIDTH, EQUAL, 10),
                text(std::to_string(u.strength).substr(0,4)) | size(WIDTH, EQUAL, 8),
            }));
        }
        auto unitsSection = vbox(unitRows) | border;

        // Conflicts
        Elements conflictElements;
        bool emGuerra = false;
        for (auto& conf : conflicts) {
            bool parte = false;
            for (auto cid : conf.attackers) if (cid == playerId_) parte = true;
            for (auto cid : conf.defenders) if (cid == playerId_) parte = true;
            if (parte) {
                emGuerra = true;
                conflictElements.push_back(
                    text("  ⚔ " + conf.name + " | Progresso: " +
                         std::to_string(conf.frontProgress).substr(0,5) +
                         " | Escalada: " + std::to_string(conf.escalationLevel).substr(0,4))
                    | color(Color::Red) | bold
                );
            }
        }
        if (!emGuerra) {
            conflictElements.push_back(text("  Sem conflitos ativos.") | dim);
        }

        return vbox({
            text(" ★ FORÇAS ARMADAS & DEFESA NACIONAL ") | bold | hcenter | color(Color::RedLight),
            forcesSection,
            hbox({
                text(" ⚔ CONFLITOS ATIVOS ") | bold | color(Color::Red),
                filler(),
            }),
            vbox(conflictElements) | border,
            hbox({
                text(" ▣ UNIDADES (" + std::to_string(units.size()) + ") ") | bold | color(Color::Yellow),
                filler(),
            }),
            unitsSection | size(HEIGHT, LESS_THAN, 12),
            separator(),
            text(" AÇÕES ") | bold | color(Color::Yellow),
            buttons->Render(),
        });
    });
}

// ============================================================
// LAWS TAB
// ============================================================

Component GameUI::createLawsTab() {
    // Category menu
    auto categoryMenu = Menu(&lawCategoryNames_, &lawCategoryIdx_);

    // Template menu
    auto templateMenu = Menu(&lawTemplateNames_, &lawTemplateIdx_);

    // Buttons
    auto btnPropose = Button("  Propor Lei Selecionada  ", [&] {
        LawCategory cats[] = {
            LawCategory::TAX, LawCategory::ECONOMIC, LawCategory::LABOR,
            LawCategory::PENSION, LawCategory::ENVIRONMENTAL, LawCategory::EDUCATION,
            LawCategory::CRIMINAL, LawCategory::IMMIGRATION, LawCategory::PUBLIC_SECURITY,
            LawCategory::CIVIL_RIGHTS, LawCategory::HEALTHCARE, LawCategory::TRADE,
            LawCategory::MILITARY, LawCategory::MEDIA, LawCategory::TECHNOLOGY,
            LawCategory::HOUSING, LawCategory::TRANSPORTATION, LawCategory::ENERGY,
            LawCategory::AGRICULTURE, LawCategory::FOREIGN_AFFAIRS,
        };
        int idx = std::clamp(lawCategoryIdx_, 0, 19);
        auto templates = engine_.getLaws().getAvailableLawTemplates(cats[idx], playerId_);
        if (lawTemplateIdx_ >= 0 && lawTemplateIdx_ < (int)templates.size()) {
            const Law& tpl = templates[lawTemplateIdx_];
            std::string blockReason = engine_.getLaws().getLawBlockReason(playerId_, tpl.name);
            if (!blockReason.empty()) {
                bool permanent = (blockReason.find("vigor")    != std::string::npos ||
                                  blockReason.find("revogada") != std::string::npos ||
                                  blockReason.find("expirada") != std::string::npos ||
                                  blockReason.find("aprovada") != std::string::npos);
                if (permanent)
                    addNotification("✗ '" + tpl.name + "' já foi promulgada — não pode ser re-proposta.");
                else
                    addNotification("✗ '" + tpl.name + "' já está " + blockReason + " no parlamento.");
            } else {
                Law lei = tpl;
                lei.country = playerId_;
                lei.proposedDate = engine_.getCurrentDate();
                LawID id = engine_.getLaws().proposeLaw(playerId_, lei);
                if (id == 0) {
                    addNotification("✗ Lei duplicada bloqueada: '" + lei.name + "'");
                } else {
                    addNotification("✓ Lei '" + lei.name + "' proposta (ID=" + std::to_string(id) + ")");
                    lawMode_ = 0;
                    updateLawTemplateNames();
                }
            }
        }
    });

    auto btnBrowse = Button(" Propor Nova Lei ", [&] {
        lawMode_ = 1;
        updateLawTemplateNames();
    });
    auto btnBack = Button(" ← Voltar ", [&] { lawMode_ = 0; });

    auto btnForceVote = Button(" Forçar Votação ", [&] {
        auto pending = engine_.getLaws().getPendingLaws(playerId_);
        if (!pending.empty()) {
            bool ok = engine_.getLaws().pushLawThroughParliament(pending[0].id);
            addNotification(ok ? "✓ Votação forçada." : "✗ Sem apoio suficiente.");
        }
    });

    auto btnRevoke = Button(" Revogar Última Lei ", [&] {
        auto active = engine_.getLaws().getActiveLaws(playerId_);
        if (!active.empty()) {
            engine_.getLaws().revokeLaw(playerId_, active.back().id);
            addNotification("✓ Lei '" + active.back().name + "' revogada.");
        }
    });

    // Overview mode
    auto overviewButtons = Container::Vertical({btnBrowse, btnForceVote, btnRevoke});

    // Propose mode
    auto proposePanel = Container::Horizontal({
        Container::Vertical({categoryMenu}) | size(WIDTH, EQUAL, 22),
        Container::Vertical({templateMenu}) | size(WIDTH, EQUAL, 36),
        Container::Vertical({btnPropose, btnBack}),
    });

    // Tab between modes
    auto modeTab = Container::Tab({overviewButtons, proposePanel}, &lawMode_);

    // Update template names when category changes
    auto component = CatchEvent(modeTab, [&](Event event) {
        // Refresh template list
        updateLawTemplateNames();
        return false;
    });

    return Renderer(component, [&, component] {
        auto activeLaws = engine_.getLaws().getActiveLaws(playerId_);
        auto pendingLaws = engine_.getLaws().getPendingLaws(playerId_);

        // Active laws list
        Elements activeElements;
        activeElements.push_back(
            text(" LEIS VIGENTES (" + std::to_string(activeLaws.size()) + ") ") |
            bold | color(Color::Green)
        );
        if (activeLaws.empty()) {
            activeElements.push_back(text("  Nenhuma lei vigente.") | dim);
        }
        for (auto& lei : activeLaws) {
            std::string catName;
            switch (lei.category) {
                case LawCategory::TAX: catName = "Tributária"; break;
                case LawCategory::ECONOMIC: catName = "Econômica"; break;
                case LawCategory::LABOR: catName = "Trabalhista"; break;
                case LawCategory::PENSION: catName = "Previdência"; break;
                case LawCategory::ENVIRONMENTAL: catName = "Ambiental"; break;
                case LawCategory::EDUCATION: catName = "Educação"; break;
                case LawCategory::CRIMINAL: catName = "Criminal"; break;
                case LawCategory::IMMIGRATION: catName = "Imigração"; break;
                case LawCategory::PUBLIC_SECURITY: catName = "Seg. Pública"; break;
                case LawCategory::CIVIL_RIGHTS: catName = "Dir. Civis"; break;
                case LawCategory::HEALTHCARE: catName = "Saúde"; break;
                case LawCategory::TRADE: catName = "Comércio"; break;
                case LawCategory::MILITARY: catName = "Militar"; break;
                case LawCategory::MEDIA: catName = "Mídia"; break;
                case LawCategory::TECHNOLOGY: catName = "Tecnologia"; break;
                case LawCategory::HOUSING: catName = "Habitação"; break;
                case LawCategory::TRANSPORTATION: catName = "Transporte"; break;
                case LawCategory::ENERGY: catName = "Energia"; break;
                case LawCategory::AGRICULTURE: catName = "Agricultura"; break;
                case LawCategory::FOREIGN_AFFAIRS: catName = "Rel. Ext."; break;
                default: catName = "Geral"; break;
            }
            activeElements.push_back(hbox({
                text("  ✓ ") | color(Color::Green),
                text(lei.name) | bold | size(WIDTH, EQUAL, 35),
                text(" [" + catName + "]") | dim,
            }));
        }

        // Pending laws list
        Elements pendingElements;
        pendingElements.push_back(
            text(" EM TRAMITAÇÃO (" + std::to_string(pendingLaws.size()) + ") ") |
            bold | color(Color::Yellow)
        );
        if (pendingLaws.empty()) {
            pendingElements.push_back(text("  Nenhuma lei em tramitação.") | dim);
        }
        for (auto& lei : pendingLaws) {
            std::string statusStr;
            Color statusColor;
            switch (lei.status) {
                case Law::Status::PROPOSED: statusStr = "PROPOSTO"; statusColor = Color::Yellow; break;
                case Law::Status::IN_COMMITTEE: statusStr = "COMITÊ"; statusColor = Color::Blue; break;
                case Law::Status::DEBATING: statusStr = "DEBATE"; statusColor = Color::Cyan; break;
                case Law::Status::VOTING: statusStr = "VOTAÇÃO"; statusColor = Color::Magenta; break;
                case Law::Status::APPROVED: statusStr = "APROVADO"; statusColor = Color::Green; break;
                default: statusStr = "???"; statusColor = Color::White; break;
            }
            double approvalImpact = lei.shortTermEffects.happiness * 0.5
                                  - lei.shortTermEffects.inflation  * 0.3
                                  - lei.shortTermEffects.unemployment * 0.3
                                  + lei.shortTermEffects.stability * 0.2;
            std::ostringstream aiss;
            aiss << std::showpos << std::fixed << std::setprecision(1) << (approvalImpact*100) << "pp";
            Color aiColor = approvalImpact >= 0.005 ? Color::GreenLight :
                            approvalImpact <= -0.005 ? Color::RedLight : Color::GrayLight;

            pendingElements.push_back(hbox({
                text("  ○ ") | color(Color::Yellow),
                text(lei.name) | size(WIDTH, EQUAL, 28),
                text(" [" + statusStr + "]") | color(statusColor) | size(WIDTH, EQUAL, 12),
                text(" Apoio: " + fmtPct(lei.parliamentarySupport)) |
                    color(lei.parliamentarySupport > 0.5 ? Color::Green : Color::Red) | size(WIDTH, EQUAL, 14),
                text(" Aprov: " + aiss.str()) | bold | color(aiColor),
            }));
        }

        Elements mainContent;
        mainContent.push_back(
            text(" ≡ LEGISLAÇÃO NACIONAL ") | bold | hcenter | color(Color::Magenta)
        );
        mainContent.push_back(separator());
        mainContent.push_back(vbox(activeElements) | border);
        mainContent.push_back(vbox(pendingElements) | border);
        mainContent.push_back(separator());

        if (lawMode_ == 0) {
            mainContent.push_back(text(" AÇÕES ") | bold | color(Color::Yellow));
            mainContent.push_back(component->Render());
        } else {
            // Show template details
            LawCategory cats[] = {
                LawCategory::TAX, LawCategory::ECONOMIC, LawCategory::LABOR,
                LawCategory::PENSION, LawCategory::ENVIRONMENTAL, LawCategory::EDUCATION,
                LawCategory::CRIMINAL, LawCategory::IMMIGRATION, LawCategory::PUBLIC_SECURITY,
                LawCategory::CIVIL_RIGHTS, LawCategory::HEALTHCARE, LawCategory::TRADE,
                LawCategory::MILITARY, LawCategory::MEDIA, LawCategory::TECHNOLOGY,
                LawCategory::HOUSING, LawCategory::TRANSPORTATION, LawCategory::ENERGY,
                LawCategory::AGRICULTURE, LawCategory::FOREIGN_AFFAIRS,
            };
            int ci = std::clamp(lawCategoryIdx_, 0, 19);
            auto templates = engine_.getLaws().getAvailableLawTemplates(cats[ci], playerId_);

            Element detailBox = text(" Selecione um template ") | dim;
            if (lawTemplateIdx_ >= 0 && lawTemplateIdx_ < (int)templates.size()) {
                auto& t = templates[lawTemplateIdx_];
                double support = engine_.getLaws().calculateLawSupport(t, playerId_);

                auto fmtEff = [](double v) -> std::string {
                    std::ostringstream ss;
                    ss << std::showpos << std::fixed << std::setprecision(1) << (v * 100) << "%";
                    return ss.str();
                };
                auto effColor = [](double v) -> Color {
                    return v > 0.001 ? Color::Green : (v < -0.001 ? Color::Red : Color::GrayLight);
                };

                detailBox = vbox({
                    text(" " + t.name + " ") | bold | color(Color::Cyan) | hcenter,
                    separator(),
                    paragraph(t.description) | dim,
                    separator(),
                    text(" EFEITOS CURTO PRAZO ") | bold | color(Color::Yellow),
                    hbox({
                        text("PIB: ") | size(WIDTH, EQUAL, 6),
                        text(fmtEff(t.shortTermEffects.gdpGrowth)) | color(effColor(t.shortTermEffects.gdpGrowth)),
                        text("  Inflação: "),
                        text(fmtEff(t.shortTermEffects.inflation)) | color(effColor(-t.shortTermEffects.inflation)),
                        text("  Desempr: "),
                        text(fmtEff(t.shortTermEffects.unemployment)) | color(effColor(-t.shortTermEffects.unemployment)),
                    }),
                    hbox({
                        text("Receita: "),
                        text(fmtEff(t.shortTermEffects.taxRevenue)) | color(effColor(t.shortTermEffects.taxRevenue)),
                        text("  Corrupção: "),
                        text(fmtEff(t.shortTermEffects.corruption)) | color(effColor(-t.shortTermEffects.corruption)),
                        text("  Felicid: "),
                        text(fmtEff(t.shortTermEffects.happiness)) | color(effColor(t.shortTermEffects.happiness)),
                    }),
                    hbox({
                        text("Estab: "),
                        text(fmtEff(t.shortTermEffects.stability)) | color(effColor(t.shortTermEffects.stability)),
                        text("  Liberdade: "),
                        text(fmtEff(t.shortTermEffects.freedom)) | color(effColor(t.shortTermEffects.freedom)),
                        text("  Poluição: "),
                        text(fmtEff(t.shortTermEffects.pollution)) | color(effColor(-t.shortTermEffects.pollution)),
                    }),
                    separator(),
                    text(" EFEITOS LONGO PRAZO ") | bold | color(Color::Cyan),
                    hbox({
                        text("PIB: "),
                        text(fmtEff(t.longTermEffects.gdpGrowth)) | color(effColor(t.longTermEffects.gdpGrowth)),
                        text("  Inflação: "),
                        text(fmtEff(t.longTermEffects.inflation)) | color(effColor(-t.longTermEffects.inflation)),
                        text("  Desempr: "),
                        text(fmtEff(t.longTermEffects.unemployment)) | color(effColor(-t.longTermEffects.unemployment)),
                    }),
                    hbox({
                        text("Receita: "),
                        text(fmtEff(t.longTermEffects.taxRevenue)) | color(effColor(t.longTermEffects.taxRevenue)),
                        text("  Corrupção: "),
                        text(fmtEff(t.longTermEffects.corruption)) | color(effColor(-t.longTermEffects.corruption)),
                        text("  Felicid: "),
                        text(fmtEff(t.longTermEffects.happiness)) | color(effColor(t.longTermEffects.happiness)),
                    }),
                    separator(),
                    // ── Impacto estimado na aprovação ──
                    text(" IMPACTO NA APROVAÇÃO DO GOVERNO ") | bold | color(Color::Yellow),
                    [&]() -> Element {
                        double impCurto = t.shortTermEffects.happiness * 0.5
                                        - t.shortTermEffects.inflation  * 0.3
                                        - t.shortTermEffects.unemployment * 0.3
                                        + t.shortTermEffects.stability * 0.2;
                        double impLongo = t.longTermEffects.happiness  * 0.5
                                        - t.longTermEffects.inflation   * 0.3
                                        - t.longTermEffects.unemployment * 0.3
                                        + t.longTermEffects.stability  * 0.2;
                        std::ostringstream sc, sl;
                        sc << std::showpos << std::fixed << std::setprecision(1) << (impCurto*100) << "pp";
                        sl << std::showpos << std::fixed << std::setprecision(1) << (impLongo*100) << "pp";
                        Color cc = impCurto >= 0.005 ? Color::GreenLight :
                                   impCurto <= -0.005 ? Color::RedLight : Color::GrayLight;
                        Color cl = impLongo >= 0.005 ? Color::GreenLight :
                                   impLongo <= -0.005 ? Color::RedLight : Color::GrayLight;
                        return hbox({
                            text("  Curto prazo: ") | dim,
                            text(sc.str()) | bold | color(cc),
                            text("   Longo prazo: ") | dim,
                            text(sl.str()) | bold | color(cl),
                        });
                    }(),
                    separator(),
                    renderGaugeBar("Apoio Público", t.publicSupport,
                                  t.publicSupport > 0.5 ? Color::Green : Color::Red),
                    renderGaugeBar("Apoio Parlam.", t.parliamentarySupport,
                                  t.parliamentarySupport > 0.5 ? Color::Green : Color::Red),
                    renderGaugeBar("Chance Aprov.", support,
                                  support > 0.5 ? Color::Green : Color::Red),
                    text("  Implementação: " + std::to_string(t.implementationDays) + " dias") | dim,
                }) | border;
            }

            mainContent.push_back(text(" PROPOR NOVA LEI ") | bold | color(Color::Green));
            mainContent.push_back(hbox({
                vbox({
                    text(" Categoria ") | bold,
                    component->Render(),
                }) | size(WIDTH, EQUAL, 60),
                detailBox | flex,
            }));
        }

        return vbox(mainContent);
    });
}

// ============================================================
// INTELLIGENCE TAB
// ============================================================

Component GameUI::createIntelligenceTab() {
    intelTargetNames_.clear();
    intelTargetIds_.clear();
    auto countryIds = engine_.getWorld().getAllCountryIds();
    for (auto cid : countryIds) {
        if (cid == playerId_) continue;
        intelTargetNames_.push_back(engine_.getWorld().getCountry(cid).name);
        intelTargetIds_.push_back(cid);
    }

    intelOpTypeNames_ = {
        "Espionagem", "Sabotagem", "Propaganda", "Desinformação",
        "Interferência Eleitoral", "Apoio a Golpe", "Ataque Cibernético",
        "Guerra Econômica",
    };

    auto targetMenu = Menu(&intelTargetNames_, &intelTargetIdx_);
    auto opTypeMenu = Menu(&intelOpTypeNames_, &intelOpTypeIdx_);

    auto btnLaunch = Button(" Lançar Operação ", [&] {
        if (intelTargetIdx_ < 0 || intelTargetIdx_ >= (int)intelTargetIds_.size()) return;
        CovertOperation::Type tipos[] = {
            CovertOperation::Type::ESPIONAGE, CovertOperation::Type::SABOTAGE,
            CovertOperation::Type::PROPAGANDA, CovertOperation::Type::DISINFORMATION,
            CovertOperation::Type::ELECTION_INTERFERENCE, CovertOperation::Type::COUP_SUPPORT,
            CovertOperation::Type::CYBER_ATTACK, CovertOperation::Type::ECONOMIC_WARFARE,
        };
        int ti = std::clamp(intelOpTypeIdx_, 0, 7);
        std::string codename = "Op-" + std::to_string(intelTargetIds_[intelTargetIdx_]) + "-" + std::to_string(ti);
        engine_.getIntelligence().launchOperation(playerId_, tipos[ti], intelTargetIds_[intelTargetIdx_], codename);
        addNotification("✓ Operação " + codename + " lançada.");
        intelMode_ = 0;
    });

    auto btnRecruitAgents = Button(" Recrutar 50 Agentes ", [&] {
        engine_.getIntelligence().recruitOperatives(playerId_, 50);
        addNotification("✓ 50 agentes recrutados.");
    });

    auto btnCounterIntel = Button(" Reforçar Contra-Intel ", [&] {
        engine_.getIntelligence().increaseCounterIntelligence(playerId_);
        addNotification("✓ Contra-inteligência reforçada.");
    });

    auto btnBudget = Button(" Aumentar Orçamento +$1B ", [&] {
        auto ag = engine_.getIntelligence().getAgency(playerId_);
        engine_.getIntelligence().setIntelBudget(playerId_, ag.budget + 1.0);
        addNotification("✓ Orçamento de inteligência aumentado.");
    });

    auto btnNewOp = Button(" Nova Operação Encoberta ", [&] { intelMode_ = 1; });
    auto btnBackIntel = Button(" ← Voltar ", [&] { intelMode_ = 0; });

    auto overviewBtns = Container::Vertical({btnNewOp, btnRecruitAgents, btnCounterIntel, btnBudget});
    auto launchPanel = Container::Vertical({
        Container::Horizontal({
            Container::Vertical({targetMenu}) | size(WIDTH, EQUAL, 20),
            Container::Vertical({opTypeMenu}) | size(WIDTH, EQUAL, 26),
        }),
        Container::Horizontal({btnLaunch, btnBackIntel}),
    });

    auto modeTab = Container::Tab({overviewBtns, launchPanel}, &intelMode_);

    return Renderer(modeTab, [&, modeTab] {
        auto agency = engine_.getIntelligence().getAgency(playerId_);
        auto ops = engine_.getIntelligence().getActiveOperations(playerId_);
        auto reports = engine_.getIntelligence().getRecentReports(playerId_, 60);

        auto agencySection = vbox({
            text(" AGÊNCIA DE INTELIGÊNCIA ") | bold | color(Color::Magenta) | hcenter,
            separator(),
            hbox({
                renderStatBox("Agência", agency.name.empty() ? "ABIN" : agency.name, Color::Cyan),
                renderStatBox("Agentes", std::to_string(agency.operativeCount), Color::Green),
                renderStatBox("Orçamento", "$" + fmtMoney(agency.budget), Color::Yellow),
            }),
            renderGaugeBar("Capacidade", agency.capability, Color::Green),
            renderGaugeBar("Contra-Intel", agency.counterIntel, Color::Blue),
            renderGaugeBar("HUMINT", agency.humanIntel, Color::Cyan),
            renderGaugeBar("SIGINT", agency.signalIntel, Color::Magenta),
            renderGaugeBar("Cyber", agency.cyberCapability, Color::Yellow),
            renderGaugeBar("Sigilo", agency.secrecy, Color::GreenLight),
        }) | border;

        // Active operations
        Elements opsElements;
        opsElements.push_back(text(" OPERAÇÕES ATIVAS (" + std::to_string(ops.size()) + ") ") |
                             bold | color(Color::Yellow));
        for (auto& op : ops) {
            auto& target = engine_.getWorld().getCountry(op.target);
            int pct = op.daysToComplete > 0 ? (op.daysElapsed * 100 / op.daysToComplete) : 100;
            std::string opTypeName;
            switch (op.type) {
                case CovertOperation::Type::ESPIONAGE: opTypeName = "Espionagem"; break;
                case CovertOperation::Type::SABOTAGE: opTypeName = "Sabotagem"; break;
                case CovertOperation::Type::PROPAGANDA: opTypeName = "Propaganda"; break;
                case CovertOperation::Type::DISINFORMATION: opTypeName = "Desinformação"; break;
                case CovertOperation::Type::ELECTION_INTERFERENCE: opTypeName = "Interf.Eleitoral"; break;
                case CovertOperation::Type::COUP_SUPPORT: opTypeName = "Apoio Golpe"; break;
                case CovertOperation::Type::CYBER_ATTACK: opTypeName = "Cyber Attack"; break;
                case CovertOperation::Type::ECONOMIC_WARFARE: opTypeName = "Guerra Econ."; break;
                default: opTypeName = "Operação"; break;
            }
            opsElements.push_back(hbox({
                text("  ► " + op.codeName) | bold | size(WIDTH, EQUAL, 18),
                text(" → " + target.name) | size(WIDTH, EQUAL, 16),
                text(" [" + opTypeName + "]") | dim | size(WIDTH, EQUAL, 20),
                text(" " + std::to_string(pct) + "%") | color(Color::Cyan),
            }));
        }
        if (ops.empty()) {
            opsElements.push_back(text("  Nenhuma operação ativa.") | dim);
        }

        // Reports
        Elements reportElements;
        reportElements.push_back(text(" RELATÓRIOS RECENTES ") | bold | color(Color::Cyan));
        int shown = 0;
        for (auto& rep : reports) {
            if (shown++ >= 5) break;
            auto& about = engine_.getWorld().getCountry(rep.about);
            reportElements.push_back(
                text("  📄 " + about.name + ": " + rep.content.substr(0, 50)) | dim
            );
        }
        if (reports.empty()) {
            reportElements.push_back(text("  Nenhum relatório recente.") | dim);
        }

        return vbox({
            text(" \u25ce SERVI\u00c7O DE INTELIG\u00caNIA ") | bold | hcenter | color(Color::MagentaLight),
            agencySection,
            vbox(opsElements) | border,
            vbox(reportElements) | border,
            separator(),
            text(" A\u00c7\u00d5ES ") | bold | color(Color::Yellow),
            modeTab->Render(),
        });
    });
}

// ============================================================
// WORLD RANKING TAB
// ============================================================

Component GameUI::createWorldRankingTab() {
    return Renderer([&] {
        struct Entry {
            CountryID id;
            std::string name;
            double gdp, pop, power, mil, hdi;
        };
        std::vector<Entry> entries;
        for (auto cid : engine_.getWorld().getAllCountryIds()) {
            auto& c = engine_.getWorld().getCountry(cid);
            entries.push_back({cid, c.name, c.gdp.billions, c.population,
                              c.calculatePowerIndex(), c.military.calculatePowerIndex(), c.hdi});
        }
        std::sort(entries.begin(), entries.end(),
                  [](const Entry& a, const Entry& b) { return a.power > b.power; });

        Elements rows;
        rows.push_back(
            hbox({
                text(" # ") | bold | size(WIDTH, EQUAL, 5),
                text("País") | bold | size(WIDTH, EQUAL, 22),
                text("PIB") | bold | size(WIDTH, EQUAL, 12),
                text("Pop") | bold | size(WIDTH, EQUAL, 10),
                text("IDH") | bold | size(WIDTH, EQUAL, 7),
                text("Mil.Power") | bold | size(WIDTH, EQUAL, 11),
                text("Poder Total") | bold | size(WIDTH, EQUAL, 13),
            }) | bgcolor(Color::GrayDark)
        );

        int rank = 1;
        for (auto& e : entries) {
            bool isPlayer = (e.id == playerId_);
            // Medalhas para top 3
            std::string rankStr;
            Color rankColor;
            if      (rank == 1) { rankStr = " ★ "; rankColor = Color::Yellow;     }
            else if (rank == 2) { rankStr = " ◆ "; rankColor = Color::GrayLight;   }
            else if (rank == 3) { rankStr = " ■ "; rankColor = Color::Red;          }
            else                { rankStr = std::string(" ") + std::to_string(rank) + " "; rankColor = Color::White; }

            Color rowColor = isPlayer ? Color::CyanLight : Color::White;
            Color gdpColor = e.gdp > 5000 ? Color::GreenLight :
                             e.gdp > 1000 ? Color::Yellow : Color::White;
            Color hdiColor = e.hdi > 0.80 ? Color::GreenLight :
                             e.hdi > 0.65 ? Color::Yellow : Color::RedLight;

            auto row = hbox({
                text(rankStr) | bold | color(rankColor) | size(WIDTH, EQUAL, 5),
                text((isPlayer ? "▸ " : "  ") + e.name) | size(WIDTH, EQUAL, 22)
                    | color(rowColor) | (isPlayer ? bold : nothing),
                text("$" + fmtMoney(e.gdp)) | color(gdpColor) | size(WIDTH, EQUAL, 12),
                text(fmtPop(e.pop)) | size(WIDTH, EQUAL, 10),
                text(std::to_string(e.hdi).substr(0, 5)) | color(hdiColor) | size(WIDTH, EQUAL, 7),
                text(std::to_string(e.mil).substr(0, 5)) | size(WIDTH, EQUAL, 11),
                text(std::to_string(e.power).substr(0, 5)) | bold | color(Color::Yellow) | size(WIDTH, EQUAL, 13),
            });
            if (isPlayer) rows.push_back(row | bgcolor(Color::GrayDark));
            else rows.push_back(row);
            rank++;
        }

        auto globalStats = hbox({
            renderStatBox("PIB Global", "$" + fmtMoney(engine_.getWorld().getGlobalGDP()), Color::Green),
            renderStatBox("Pop Global", fmtPop(engine_.getWorld().getGlobalPopulation()), Color::Cyan),
            renderStatBox("CO₂ Global", fmtNum(engine_.getWorld().getGlobalCO2()) + "MT", Color::Red),
            renderStatBox("Temp Δ", std::to_string(engine_.getWorld().getGlobalTemperatureAnomaly()).substr(0,4) + "°C", Color::Yellow),
        });

        return vbox({
            text(" ☰ RANKING MUNDIAL DE PODÊNIO ") | bold | hcenter | color(Color::Yellow),
            separator(),
            globalStats,
            separator(),
            vbox(rows) | border,
        });
    });
}

// ============================================================
// MAIN RUN LOOP
// ============================================================

void GameUI::run() {
    auto screen = ScreenInteractive::Fullscreen();

    // Tab navigation
    auto tabMenu = Menu(&tabNames_, &selectedTab_, MenuOption::HorizontalAnimated());

    // Create tab contents
    auto tabContent = Container::Tab({
        createDashboardTab(),
        createEconomyTab(),
        createDiplomacyTab(),
        createMilitaryTab(),
        createLawsTab(),
        createIntelligenceTab(),
        createWorldRankingTab(),
    }, &selectedTab_);

    // Time control buttons
    auto btnAdvWeek = Button(" [W] +1 Semana ", [&] {
        engine_.forceAdvance(24 * 7);
        auto d = engine_.getCurrentDate();
        addNotification("✓ Avançado 1 semana → " + fmtDate(d));
    });
    auto btnAdvMonth = Button(" [M] +1 Mês ", [&] {
        engine_.forceAdvance(24 * 30);
        auto d = engine_.getCurrentDate();
        addNotification("✓ Avançado 1 mês → " + fmtDate(d));
    });
    auto btnAdvYear = Button(" [Y] +1 Ano ", [&] {
        engine_.forceAdvance(24 * 365);
        auto d = engine_.getCurrentDate();
        addNotification("✓ Avançado 1 ano → " + fmtDate(d));
    });
    auto btnQuit = Button(" [Q] Sair ", [&] { screen.Exit(); });

    auto timeControls = Container::Horizontal({
        btnAdvWeek, btnAdvMonth, btnAdvYear, btnQuit,
    });

    // Main layout
    auto mainContainer = Container::Vertical({
        tabMenu,
        timeControls,
        tabContent,
    });

    // Keyboard shortcut handler
    auto mainComponent = CatchEvent(mainContainer, [&](Event event) {
        if (event == Event::Character('w') || event == Event::Character('W')) {
            engine_.forceAdvance(24 * 7);
            addNotification("✓ Avançado 1 semana → " + fmtDate(engine_.getCurrentDate()));
            return true;
        }
        if (event == Event::Character('m') || event == Event::Character('M')) {
            engine_.forceAdvance(24 * 30);
            addNotification("✓ Avançado 1 mês → " + fmtDate(engine_.getCurrentDate()));
            return true;
        }
        if (event == Event::Character('y') || event == Event::Character('Y')) {
            engine_.forceAdvance(24 * 365);
            addNotification("✓ Avançado 1 ano → " + fmtDate(engine_.getCurrentDate()));
            return true;
        }
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            screen.Exit();
            return true;
        }
        return false;
    });

    // Final renderer
    auto renderer = Renderer(mainComponent, [&] {
        return vbox({
            renderHeader(),
            hbox({
                tabMenu->Render() | flex,
                separator(),
                timeControls->Render(),
            }) | border,
            tabContent->Render() | flex | frame,
            renderStatusBar(),
        });
    });

    // ── Banco Central: atualização automática de juros a cada 3 meses ──
    engine_.onNewMonth([&](const SimDate&) {
        centralBankMonthCounter_++;
        if (centralBankMonthCounter_ % 3 == 0) {
            auto& co = engine_.getWorld().getCountry(playerId_);
            // Regra de Taylor simplificada
            double inflGap    = co.inflation - 0.03;          // meta 3%
            double growthGap  = co.gdpGrowthRate - 0.025;     // potencial 2.5%
            double unempGap   = co.unemploymentRate - 0.05;   // NAIRU ~5%

            double targetRate = 0.025
                + 1.5  * inflGap           // foco em inflação
                - 0.50 * growthGap         // suporte ao crescimento
                + 0.30 * unempGap;         // pressão por emprego

            targetRate = std::clamp(targetRate, 0.005, 0.28);

            // Ajuste gradual máximo ±0.5pp por trimestre
            double current = co.interestRate;
            double delta   = std::clamp(targetRate - current, -0.005, 0.005);
            double newRate = std::clamp(current + delta, 0.005, 0.28);

            engine_.getEconomy().setInterestRate(playerId_, newRate);

            std::ostringstream msg;
            msg << "🏦 Banco Central: juros ";
            if (delta > 0.0002)      msg << "▲ subiu para ";
            else if (delta < -0.0002) msg << "▼ cortou para ";
            else                      msg << "manteve em ";
            msg << std::fixed << std::setprecision(2) << (newRate * 100.0) << "%";
            msg << "  (inflação " << std::fixed << std::setprecision(1) << (co.inflation*100) << "%)";
            addNotification(msg.str());
        }
    });

    screen.Loop(renderer);
}

} // namespace GPS
