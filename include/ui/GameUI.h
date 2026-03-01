/**
 * GPS - Geopolitical Simulator
 * GameUI.h - Interface gráfica terminal com FTXUI
 */
#pragma once

#include "core/SimulationEngine.h"
#include "core/Types.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <string>
#include <vector>
#include <functional>

namespace GPS {

class GameUI {
public:
    explicit GameUI(SimulationEngine& engine);
    void run();

private:
    // Tab creation
    ftxui::Component createDashboardTab();
    ftxui::Component createEconomyTab();
    ftxui::Component createDiplomacyTab();
    ftxui::Component createMilitaryTab();
    ftxui::Component createLawsTab();
    ftxui::Component createIntelligenceTab();
    ftxui::Component createWorldRankingTab();

    // Rendering helpers
    ftxui::Element renderHeader();
    ftxui::Element renderStatusBar();
    ftxui::Element renderStatBox(const std::string& label, const std::string& value,
                                  ftxui::Color color = ftxui::Color::White);
    ftxui::Element renderGaugeBar(const std::string& label, double value,
                                   ftxui::Color color, int labelWidth = 16);
    ftxui::Element renderDualGauge(const std::string& label, double value,
                                    ftxui::Color posColor, ftxui::Color negColor);

    // Formatting helpers
    std::string fmtMoney(double billions) const;
    std::string fmtPct(double ratio) const;
    std::string fmtNum(double num) const;
    std::string fmtDate(const SimDate& date) const;
    std::string fmtPop(double millions) const;

    // Engine reference
    SimulationEngine& engine_;
    CountryID playerId_;

    // Tab navigation
    int selectedTab_ = 0;
    std::vector<std::string> tabNames_;

    // Economy sub-tabs (0=Panorama,1=Impostos,2=Gastos,3=Defesa..14=Serv.Dívida)
    int ecoMode_ = 0;
    std::vector<std::string> ecoSubTabNames_;
    int taxLow_ = 8, taxMid_ = 15, taxHigh_ = 28;
    int taxCorp_ = 34, taxSales_ = 17, taxImport_ = 5;
    // Orçamento por categoria (% do orçamento, 0-60)
    int budDefense_ = 10, budEducation_ = 14, budHealthcare_ = 12;
    int budInfra_ = 7,    budSocial_ = 18,   budScience_ = 5;
    int budEnv_ = 3,      budAdmin_ = 6,     budDebt_ = 10;
    int budSecurity_ = 6, budAgriculture_ = 4, budCulture_ = 2;
    int budTransport_ = 3;
    int spendingRatio_ = 22; // gastos como % do PIB (10–55)
    // Banco Central — juros automáticos a cada 3 meses
    int centralBankMonthCounter_ = 0;

    // Diplomacy
    int diploSelected_ = 0;
    int diploAction_ = 0;
    std::vector<std::string> diploCountryNames_;
    std::vector<CountryID>   diploOtherIds_;

    // Military
    int milMode_ = 0;

    // Laws
    int lawMode_ = 0;  // 0=overview, 1=propose
    int lawCategoryIdx_ = 0;
    int lawTemplateIdx_ = 0;
    std::vector<std::string> lawCategoryNames_;
    std::vector<std::string> lawTemplateNames_;
    void updateLawTemplateNames();

    // Intelligence
    int intelMode_ = 0;
    int intelTargetIdx_ = 0;
    int intelOpTypeIdx_ = 0;
    std::vector<std::string> intelTargetNames_;
    std::vector<CountryID>   intelTargetIds_;
    std::vector<std::string> intelOpTypeNames_;

    // Notifications
    std::vector<std::string> notifications_;
    void addNotification(const std::string& msg);

    // Speed control
    int speedIdx_ = 0;
};

} // namespace GPS
