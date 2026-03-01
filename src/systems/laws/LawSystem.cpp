/**
 * GPS - Geopolitical Simulator
 * LawSystem.cpp - Matches LawSystem.h exactly
 */

#include "systems/laws/LawSystem.h"
#include <iostream>
#include <algorithm>

namespace GPS {

LawSystem::LawSystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void LawSystem::init() {
    std::cout << "[Laws] Initializing legal system..." << std::endl;

    // ==================== TRIBUTÁRIA (TAX) ====================
    {
        Law t; t.name = "Imposto de Renda Progressivo";
        t.description = "Estabelece faixas progressivas de IR: isenta até 2 SM, 15% até 5 SM, 27.5% acima. Redistribui renda mas pode desincentivar trabalho qualificado.";
        t.category = LawCategory::TAX;
        t.shortTermEffects.gdpGrowth = -0.01; t.shortTermEffects.inequality = -0.03;
        t.shortTermEffects.taxRevenue = 0.05; t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.gdpGrowth = 0.0; t.longTermEffects.inequality = -0.05;
        t.longTermEffects.taxRevenue = 0.07;
        t.implementationDays = 90; t.publicSupport = 0.55; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(2.0); t.yearlyMaintenanceCost = Money(0.5);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.4;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.5;
        t.groupImpact[SocialGroupType::WORKERS] = 0.3;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Imposto sobre Grandes Fortunas";
        t.description = "Taxa anual de 1-3% sobre patrimônio acima de R$50M. Combate concentração de renda mas pode provocar fuga de capitais. Estima-se arrecadação de R$40-80B/ano, porém evasão fiscal pode reduzir efetividade em até 60%.";
        t.category = LawCategory::TAX;
        t.shortTermEffects.taxRevenue = 0.03; t.shortTermEffects.inequality = -0.04;
        t.shortTermEffects.gdpGrowth = -0.015;
        t.longTermEffects.inequality = -0.06; t.longTermEffects.taxRevenue = 0.02;
        t.longTermEffects.gdpGrowth = -0.01;
        t.implementationDays = 120; t.publicSupport = 0.60; t.parliamentarySupport = 0.35;
        t.implementationCost = Money(1.5); t.yearlyMaintenanceCost = Money(0.3);
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.8;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.5;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.6;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Reforma Tributária Simplificada";
        t.description = "Unifica impostos sobre consumo em IVA único com alíquota de 25%. Reduz de 5 tributos federais para 1. Período de transição de 8 anos. Reduz burocracia mas exige reformulação completa do sistema contábil nacional.";
        t.category = LawCategory::TAX;
        t.shortTermEffects.gdpGrowth = 0.01; t.shortTermEffects.taxRevenue = -0.02;
        t.shortTermEffects.corruption = -0.02;
        t.longTermEffects.gdpGrowth = 0.03; t.longTermEffects.taxRevenue = 0.04;
        t.longTermEffects.corruption = -0.04;
        t.implementationDays = 365; t.publicSupport = 0.65; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(15.0); t.yearlyMaintenanceCost = Money(2.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.5;
        t.groupImpact[SocialGroupType::PUBLIC_SERVANTS] = -0.3;
        t.groupImpact[SocialGroupType::WORKERS] = 0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Isenção Fiscal para Startups";
        t.description = "Empresas de tecnologia com menos de 5 anos recebem isenção de impostos corporativos. Fomenta inovação e atrai investidores, mas reduz arrecadação de curto prazo e pode criar distorções competitivas.";
        t.category = LawCategory::TAX;
        t.shortTermEffects.gdpGrowth = 0.008; t.shortTermEffects.taxRevenue = -0.01;
        t.shortTermEffects.unemployment = -0.005;
        t.longTermEffects.gdpGrowth = 0.02; t.longTermEffects.taxRevenue = 0.01;
        t.longTermEffects.education = 0.01;
        t.implementationDays = 60; t.publicSupport = 0.50; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(0.5); t.yearlyMaintenanceCost = Money(0.2);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.7;
        t.groupImpact[SocialGroupType::STUDENTS] = 0.3;
        t.groupImpact[SocialGroupType::WORKERS] = 0.1;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Tributação de Dividendos";
        t.description = "Imposto de 20% sobre distribuição de lucros e dividendos para pessoas físicas. Fecha brecha tributária usada por classe alta para evitar IR, mas pode desestimular investimento em ações.";
        t.category = LawCategory::TAX;
        t.shortTermEffects.taxRevenue = 0.03; t.shortTermEffects.inequality = -0.02;
        t.shortTermEffects.gdpGrowth = -0.005;
        t.longTermEffects.taxRevenue = 0.04; t.longTermEffects.inequality = -0.03;
        t.implementationDays = 90; t.publicSupport = 0.55; t.parliamentarySupport = 0.35;
        t.implementationCost = Money(0.8);
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.6;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== ECONÔMICA (ECONOMIC) ====================
    {
        Law t; t.name = "Privatização de Empresas Estatais";
        t.description = "Venda de empresas públicas para o setor privado via leilões na bolsa de valores. Inclui golden shares para setores estratégicos. Gera receita de R$200-500B mas reduz capacidade do Estado de intervir na economia e pode causar demissões em massa.";
        t.category = LawCategory::ECONOMIC;
        t.shortTermEffects.gdpGrowth = 0.02; t.shortTermEffects.taxRevenue = 0.10;
        t.shortTermEffects.unemployment = 0.02; t.shortTermEffects.happiness = -0.02;
        t.longTermEffects.gdpGrowth = 0.03; t.longTermEffects.unemployment = -0.01;
        t.implementationDays = 180; t.publicSupport = 0.35; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(5.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.6;
        t.groupImpact[SocialGroupType::PUBLIC_SERVANTS] = -0.7;
        t.groupImpact[SocialGroupType::UNIONS] = -0.6;
        t.groupImpact[SocialGroupType::WORKERS] = -0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Programa Nacional de Industrialização";
        t.description = "Incentivos fiscais e crédito subsidiado para indústrias nacionais. Fortalece produção interna mas aumenta gasto público.";
        t.category = LawCategory::ECONOMIC;
        t.shortTermEffects.gdpGrowth = 0.015; t.shortTermEffects.governmentSpending = 0.04;
        t.shortTermEffects.unemployment = -0.02;
        t.longTermEffects.gdpGrowth = 0.04; t.longTermEffects.unemployment = -0.03;
        t.implementationDays = 270; t.publicSupport = 0.55; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(20.0); t.yearlyMaintenanceCost = Money(8.0);
        t.groupImpact[SocialGroupType::WORKERS] = 0.4;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.5;
        t.groupImpact[SocialGroupType::UNIONS] = 0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Desregulamentação Econômica";
        t.description = "Reduz regulações sobre negócios, facilitando abertura de empresas e investimento. Aumenta crescimento mas pode ampliar desigualdade.";
        t.category = LawCategory::ECONOMIC;
        t.shortTermEffects.gdpGrowth = 0.02; t.shortTermEffects.inequality = 0.02;
        t.shortTermEffects.corruption = 0.01;
        t.longTermEffects.gdpGrowth = 0.03; t.longTermEffects.inequality = 0.03;
        t.longTermEffects.pollution = 0.02;
        t.implementationDays = 90; t.publicSupport = 0.40; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(1.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.7;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = 0.5;
        t.groupImpact[SocialGroupType::WORKERS] = -0.3;
        t.groupImpact[SocialGroupType::UNIONS] = -0.5;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Controle de Preços e Aluguéis";
        t.description = "Tabelamento de preços de itens essenciais e limitação de reajustes de aluguel. Popular mas pode gerar escassez e mercado negro.";
        t.category = LawCategory::ECONOMIC;
        t.shortTermEffects.happiness = 0.04; t.shortTermEffects.inflation = -0.03;
        t.shortTermEffects.gdpGrowth = -0.02;
        t.longTermEffects.gdpGrowth = -0.03; t.longTermEffects.inflation = 0.02;
        t.implementationDays = 30; t.publicSupport = 0.70; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(0.5);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.5;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Zona Econômica Especial";
        t.description = "Criação de zonas com impostos reduzidos, burocracia mínima e incentivos para empresas estrangeiras. Modelo semelhante a Shenzhen/Dubai. Atrai investimento estrangeiro direto e gera empregos qualificados mas cria enclaves de desigualdade.";
        t.category = LawCategory::ECONOMIC;
        t.shortTermEffects.gdpGrowth = 0.015; t.shortTermEffects.taxRevenue = -0.01;
        t.shortTermEffects.unemployment = -0.01;
        t.longTermEffects.gdpGrowth = 0.035; t.longTermEffects.inequality = 0.02;
        t.longTermEffects.taxRevenue = 0.02;
        t.implementationDays = 365; t.publicSupport = 0.45; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(30.0); t.yearlyMaintenanceCost = Money(5.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.6;
        t.groupImpact[SocialGroupType::WORKERS] = 0.3;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = -0.1;
        lawTemplates_.push_back(t);
    }

    // ==================== TRABALHISTA (LABOR) ====================
    {
        Law t; t.name = "Reforma Trabalhista Flexível";
        t.description = "Flexibiliza contratação e demissão, permite trabalho intermitente e terceirização irrestrita. Reduz custos empresariais mas precariza emprego.";
        t.category = LawCategory::LABOR;
        t.shortTermEffects.gdpGrowth = 0.015; t.shortTermEffects.unemployment = -0.02;
        t.shortTermEffects.happiness = -0.03; t.shortTermEffects.inequality = 0.02;
        t.longTermEffects.gdpGrowth = 0.02; t.longTermEffects.unemployment = -0.03;
        t.longTermEffects.inequality = 0.03;
        t.implementationDays = 120; t.publicSupport = 0.30; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(2.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.7;
        t.groupImpact[SocialGroupType::WORKERS] = -0.5;
        t.groupImpact[SocialGroupType::UNIONS] = -0.8;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Lei de Direitos Trabalhistas Ampliados";
        t.description = "Amplia licença maternidade/paternidade para 6 meses, garante 30 dias de férias remuneradas, seguro-desemprego de 12 meses e participação nos lucros obrigatória. Melhora qualidade de vida mas aumenta custos empresariais em 15-25%.";
        t.category = LawCategory::LABOR;
        t.shortTermEffects.gdpGrowth = -0.01; t.shortTermEffects.happiness = 0.04;
        t.shortTermEffects.inequality = -0.02; t.shortTermEffects.governmentSpending = 0.02;
        t.longTermEffects.happiness = 0.06; t.longTermEffects.gdpGrowth = 0.01;
        t.implementationDays = 150; t.publicSupport = 0.65; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(3.0); t.yearlyMaintenanceCost = Money(10.0);
        t.groupImpact[SocialGroupType::WORKERS] = 0.8;
        t.groupImpact[SocialGroupType::UNIONS] = 0.7;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.5;
        t.groupImpact[SocialGroupType::WOMEN] = 0.6;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Salário Mínimo Nacional Elevado";
        t.description = "Aumento de 40% do salário mínimo com reajustes anuais acima da inflação + PIB. Beneficia 50M de trabalhadores mas pode aumentar desemprego formal em 2-5% e informalidade.";
        t.category = LawCategory::LABOR;
        t.shortTermEffects.happiness = 0.03; t.shortTermEffects.inequality = -0.03;
        t.shortTermEffects.unemployment = 0.01; t.shortTermEffects.inflation = 0.01;
        t.longTermEffects.inequality = -0.04; t.longTermEffects.happiness = 0.04;
        t.implementationDays = 60; t.publicSupport = 0.70; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(1.0); t.yearlyMaintenanceCost = Money(15.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.7;
        t.groupImpact[SocialGroupType::WORKERS] = 0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.4;
        t.groupImpact[SocialGroupType::UNEMPLOYED] = -0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Jornada de Trabalho de 4 Dias";
        t.description = "Reduz semana de trabalho para 32 horas em 4 dias sem redução salarial. Estudos mostram produtividade igual ou superior, mas indústria e serviços 24h enfrentam dificuldades de adaptação.";
        t.category = LawCategory::LABOR;
        t.shortTermEffects.happiness = 0.05; t.shortTermEffects.gdpGrowth = -0.01;
        t.shortTermEffects.unemployment = -0.01;
        t.longTermEffects.happiness = 0.07; t.longTermEffects.gdpGrowth = 0.005;
        t.longTermEffects.healthcare = 0.01;
        t.implementationDays = 365; t.publicSupport = 0.75; t.parliamentarySupport = 0.35;
        t.implementationCost = Money(0.5);
        t.groupImpact[SocialGroupType::WORKERS] = 0.9;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.4;
        t.groupImpact[SocialGroupType::UNIONS] = 0.6;
        lawTemplates_.push_back(t);
    }

    // ==================== PREVIDÊNCIA (PENSION) ====================
    {
        Law t; t.name = "Reforma da Previdência";
        t.description = "Idade mínima de 65 anos (homens) e 62 (mulheres), tempo mínimo de contribuição de 25 anos, alíquotas progressivas de 7.5% a 14%. Economia estimada de R$800B em 10 anos. Extremamente impopular com trabalhadores e sindicatos.";
        t.category = LawCategory::PENSION;
        t.shortTermEffects.happiness = -0.05; t.shortTermEffects.stability = -0.02;
        t.shortTermEffects.governmentSpending = -0.04;
        t.longTermEffects.governmentSpending = -0.08; t.longTermEffects.stability = 0.02;
        t.longTermEffects.gdpGrowth = 0.01;
        t.implementationDays = 365; t.publicSupport = 0.25; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(3.0); t.yearlyMaintenanceCost = Money(0.5);
        t.groupImpact[SocialGroupType::RETIREES] = -0.8;
        t.groupImpact[SocialGroupType::WORKERS] = -0.5;
        t.groupImpact[SocialGroupType::UNIONS] = -0.7;
        t.groupImpact[SocialGroupType::ELDERLY] = -0.6;
        t.groupImpact[SocialGroupType::YOUTH] = 0.2;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Previdência Mista (Capitalização)";
        t.description = "Cria pilar de capitalização individual complementar ao regime público. Trabalhadores podem destinar parte da contribuição a fundos privados de pensão. Reduz pressão fiscal futura mas cria riscos de mercado para aposentados.";
        t.category = LawCategory::PENSION;
        t.shortTermEffects.happiness = -0.02; t.shortTermEffects.governmentSpending = -0.02;
        t.shortTermEffects.stability = -0.01;
        t.longTermEffects.governmentSpending = -0.05; t.longTermEffects.gdpGrowth = 0.015;
        t.longTermEffects.inequality = 0.02;
        t.implementationDays = 365*2; t.publicSupport = 0.35; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(8.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::UPPER_MIDDLE_CLASS] = 0.3;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = -0.3;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Renda Básica Universal";
        t.description = "Pagamento mensal de R$600 a todos os cidadãos maiores de 18 anos sem renda formal. Custo estimado de R$200B/ano. Elimina pobreza extrema e estimula consumo, mas pressiona fortemente as contas públicas e pode desincentivar trabalho formal.";
        t.category = LawCategory::PENSION;
        t.shortTermEffects.happiness = 0.06; t.shortTermEffects.inequality = -0.05;
        t.shortTermEffects.governmentSpending = 0.08; t.shortTermEffects.gdpGrowth = 0.01;
        t.longTermEffects.inequality = -0.08; t.longTermEffects.happiness = 0.08;
        t.longTermEffects.governmentSpending = 0.10;
        t.implementationDays = 365; t.publicSupport = 0.55; t.parliamentarySupport = 0.30;
        t.implementationCost = Money(25.0); t.yearlyMaintenanceCost = Money(200.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.9;
        t.groupImpact[SocialGroupType::UNEMPLOYED] = 0.8;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.5;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.3;
        t.groupImpact[SocialGroupType::YOUTH] = 0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Bolsa Família Expandida";
        t.description = "Expansão do programa de transferência condicionada de renda: R$800/mês para famílias com renda per capita até meio salário mínimo, condicionado à frequência escolar e vacinação. Atende 20M de famílias.";
        t.category = LawCategory::PENSION;
        t.shortTermEffects.happiness = 0.04; t.shortTermEffects.inequality = -0.04;
        t.shortTermEffects.governmentSpending = 0.05; t.shortTermEffects.education = 0.01;
        t.longTermEffects.inequality = -0.06; t.longTermEffects.happiness = 0.05;
        t.longTermEffects.education = 0.02; t.longTermEffects.healthcare = 0.01;
        t.implementationDays = 180; t.publicSupport = 0.65; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(80.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.8;
        t.groupImpact[SocialGroupType::FARMERS] = 0.3;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== AMBIENTAL (ENVIRONMENTAL) ====================
    {
        Law t; t.name = "Proteção Ambiental Rigorosa";
        t.description = "Proíbe desmatamento em biomas protegidos (Amazônia, Cerrado, Mata Atlântica), multas de até R$50M para poluição industrial e exige licenciamento ambiental rigoroso com prazo de 2 anos. Cria fundo de R$10B para recuperação de áreas degradadas.";
        t.category = LawCategory::ENVIRONMENTAL;
        t.shortTermEffects.gdpGrowth = -0.02; t.shortTermEffects.pollution = -0.05;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.pollution = -0.10; t.longTermEffects.healthcare = 0.02;
        t.longTermEffects.happiness = 0.03;
        t.implementationDays = 150; t.publicSupport = 0.50; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(10.0); t.yearlyMaintenanceCost = Money(5.0);
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = 0.9;
        t.groupImpact[SocialGroupType::FARMERS] = -0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.3;
        t.groupImpact[SocialGroupType::INDIGENOUS] = 0.7;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Mercado de Carbono Nacional";
        t.description = "Cria sistema cap-and-trade para emissões de CO2 com 300 empresas participantes. Teto de emissões reduz 5% ao ano. Créditos negociáveis em bolsa com preço-piso de R$50/tonCO2. Incentiva transição verde com impacto econômico moderado.";
        t.category = LawCategory::ENVIRONMENTAL;
        t.shortTermEffects.pollution = -0.03; t.shortTermEffects.gdpGrowth = -0.01;
        t.shortTermEffects.taxRevenue = 0.02;
        t.longTermEffects.pollution = -0.08; t.longTermEffects.gdpGrowth = 0.01;
        t.longTermEffects.taxRevenue = 0.03;
        t.implementationDays = 270; t.publicSupport = 0.45; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(1.5);
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = 0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Desregulamentação Ambiental";
        t.description = "Flexibiliza licenciamento ambiental (prazo máximo de 60 dias), reduz áreas de proteção em 40%, permite mineração em terras indígenas. Acelera projetos econômicos mas causa dano ambiental significativo e sanções internacionais.";
        t.category = LawCategory::ENVIRONMENTAL;
        t.shortTermEffects.gdpGrowth = 0.03; t.shortTermEffects.pollution = 0.05;
        t.shortTermEffects.happiness = -0.02;
        t.longTermEffects.pollution = 0.08; t.longTermEffects.healthcare = -0.03;
        t.longTermEffects.happiness = -0.04;
        t.implementationDays = 60; t.publicSupport = 0.30; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(0.5);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.6;
        t.groupImpact[SocialGroupType::FARMERS] = 0.5;
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = -0.9;
        t.groupImpact[SocialGroupType::INDIGENOUS] = -0.8;
        lawTemplates_.push_back(t);
    }

    // ==================== EDUCAÇÃO (EDUCATION) ====================
    {
        Law t; t.name = "Reforma Educacional Abrangente";
        t.description = "Reestrutura Base Nacional Curricular com ênfase em STEM e pensamento crítico. Piso salarial de R$8000 para professores, ensino integral em 100% das escolas públicas, e programa nacional de ensino técnico em parceria com indústria. Custo de R$80B/ano adicional.";
        t.category = LawCategory::EDUCATION;
        t.shortTermEffects.governmentSpending = 0.04; t.shortTermEffects.education = 0.02;
        t.shortTermEffects.happiness = 0.02;
        t.longTermEffects.education = 0.08; t.longTermEffects.gdpGrowth = 0.02;
        t.longTermEffects.corruption = -0.02;
        t.implementationDays = 365; t.publicSupport = 0.75; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(15.0); t.yearlyMaintenanceCost = Money(80.0);
        t.groupImpact[SocialGroupType::STUDENTS] = 0.8;
        t.groupImpact[SocialGroupType::TEACHERS] = 0.9;
        t.groupImpact[SocialGroupType::YOUTH] = 0.6;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Universidade Pública Gratuita e Universal";
        t.description = "Garante acesso gratuito ao ensino superior público com vagas expandidas em 50%. Cria 30 novos campi federais. Alta qualificação da força de trabalho a longo prazo, mas custo anual adicional de R$40B.";
        t.category = LawCategory::EDUCATION;
        t.shortTermEffects.governmentSpending = 0.05; t.shortTermEffects.education = 0.01;
        t.shortTermEffects.happiness = 0.03;
        t.longTermEffects.education = 0.06; t.longTermEffects.gdpGrowth = 0.03;
        t.longTermEffects.inequality = -0.03;
        t.implementationDays = 365*2; t.publicSupport = 0.70; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(30.0); t.yearlyMaintenanceCost = Money(40.0);
        t.groupImpact[SocialGroupType::STUDENTS] = 0.9;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.6;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Vouchers Educacionais";
        t.description = "Distribui vouchers de R$1500/mês para pais escolherem escola (pública ou privada). Estimula competição entre escolas mas pode aprofundar segregação educacional entre ricos e pobres.";
        t.category = LawCategory::EDUCATION;
        t.shortTermEffects.education = 0.01; t.shortTermEffects.inequality = 0.01;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.education = 0.03; t.longTermEffects.inequality = 0.02;
        t.implementationDays = 180; t.publicSupport = 0.45; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(35.0);
        t.groupImpact[SocialGroupType::UPPER_MIDDLE_CLASS] = 0.5;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = -0.2;
        t.groupImpact[SocialGroupType::TEACHERS] = -0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== CRIMINAL (CRIMINAL) ====================
    {
        Law t; t.name = "Endurecimento Penal";
        t.description = "Aumenta penas de crimes violentos em 50%, restringe progressão para crimes hediondos, amplia rol de crimes inafiançáveis e reduz idade penal para 16 anos. Reduz criminalidade mas superlota prisões (atual: 180% da capacidade).";
        t.category = LawCategory::CRIMINAL;
        t.shortTermEffects.criminalRate = -0.03; t.shortTermEffects.freedom = -0.03;
        t.shortTermEffects.governmentSpending = 0.02; t.shortTermEffects.stability = 0.02;
        t.longTermEffects.criminalRate = -0.02; t.longTermEffects.freedom = -0.04;
        t.implementationDays = 90; t.publicSupport = 0.60; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(12.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = -0.3;
        t.groupImpact[SocialGroupType::UPPER_MIDDLE_CLASS] = 0.4;
        t.groupImpact[SocialGroupType::YOUTH] = -0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Descriminalização de Drogas Leves";
        t.description = "Descriminaliza posse de até 40g de maconha para uso pessoal. Legaliza cultivo doméstico de 6 plantas. Cria regulamentação para venda medicinal. Reduz encarceramento em massa (40% dos presos por tráfico de pequenas quantidades) e libera R$10B/ano em recursos policiais.";
        t.category = LawCategory::CRIMINAL;
        t.shortTermEffects.criminalRate = -0.01; t.shortTermEffects.freedom = 0.03;
        t.shortTermEffects.happiness = 0.01; t.shortTermEffects.taxRevenue = 0.01;
        t.longTermEffects.criminalRate = -0.03; t.longTermEffects.freedom = 0.04;
        t.longTermEffects.governmentSpending = -0.02;
        t.implementationDays = 90; t.publicSupport = 0.40; t.parliamentarySupport = 0.35;
        t.implementationCost = Money(1.0);
        t.groupImpact[SocialGroupType::YOUTH] = 0.5;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.4;
        t.groupImpact[SocialGroupType::RELIGIOUS] = -0.6;
        t.groupImpact[SocialGroupType::ELDERLY] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Justiça Restaurativa";
        t.description = "Prioriza mediação, serviço comunitário e reparação do dano sobre punição para crimes não-violentos. Reduz reincidência de 70% para 30% e custos do sistema penal em 40%. Programas de reintegração social obrigatórios.";
        t.category = LawCategory::CRIMINAL;
        t.shortTermEffects.criminalRate = 0.01; t.shortTermEffects.happiness = 0.01;
        t.shortTermEffects.governmentSpending = -0.01;
        t.longTermEffects.criminalRate = -0.04; t.longTermEffects.happiness = 0.03;
        t.longTermEffects.governmentSpending = -0.03;
        t.implementationDays = 180; t.publicSupport = 0.45; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(3.0); t.yearlyMaintenanceCost = Money(2.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.4;
        t.groupImpact[SocialGroupType::YOUTH] = 0.3;
        t.groupImpact[SocialGroupType::UPPER_MIDDLE_CLASS] = -0.2;
        lawTemplates_.push_back(t);
    }

    // ==================== IMIGRAÇÃO (IMMIGRATION) ====================
    {
        Law t; t.name = "Política de Imigração Aberta";
        t.description = "Facilita vistos de trabalho e residência, acelera naturalização. Atrai mão de obra qualificada e aumenta diversidade cultural.";
        t.category = LawCategory::IMMIGRATION;
        t.shortTermEffects.immigration = 0.05; t.shortTermEffects.gdpGrowth = 0.01;
        t.shortTermEffects.happiness = -0.01;
        t.longTermEffects.gdpGrowth = 0.02; t.longTermEffects.immigration = 0.08;
        t.longTermEffects.stability = -0.01;
        t.implementationDays = 90; t.publicSupport = 0.35; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(2.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::IMMIGRANTS] = 0.8;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.4;
        t.groupImpact[SocialGroupType::WORKERS] = -0.2;
        t.groupImpact[SocialGroupType::NATIONALISTS] = -0.7;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Controle Migratório Rigoroso";
        t.description = "Restringe imigração, aumenta fiscalização de fronteiras com radar e drones, deportação acelerada de ilegais. Muro digital em 3000km de fronteira. Reduz fluxo mas impacta mão de obra barata e relações com vizinhos.";
        t.category = LawCategory::IMMIGRATION;
        t.shortTermEffects.immigration = -0.05; t.shortTermEffects.stability = 0.02;
        t.shortTermEffects.freedom = -0.02; t.shortTermEffects.governmentSpending = 0.02;
        t.longTermEffects.immigration = -0.08; t.longTermEffects.unemployment = 0.01;
        t.longTermEffects.gdpGrowth = -0.01;
        t.implementationDays = 120; t.publicSupport = 0.50; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(15.0); t.yearlyMaintenanceCost = Money(8.0);
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.8;
        t.groupImpact[SocialGroupType::IMMIGRANTS] = -0.9;
        t.groupImpact[SocialGroupType::FARMERS] = -0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== SEGURANÇA PÚBLICA (PUBLIC_SECURITY) ====================
    {
        Law t; t.name = "Lei de Segurança Nacional";
        t.description = "Amplia poderes das forças de segurança, vigilância eletrônica em massa e restrições a manifestações. Eficaz contra crime mas limita liberdades.";
        t.category = LawCategory::PUBLIC_SECURITY;
        t.shortTermEffects.freedom = -0.05; t.shortTermEffects.stability = 0.05;
        t.shortTermEffects.criminalRate = -0.04; t.shortTermEffects.happiness = -0.02;
        t.longTermEffects.freedom = -0.06; t.longTermEffects.stability = 0.08;
        t.longTermEffects.criminalRate = -0.05;
        t.implementationDays = 60; t.publicSupport = 0.45; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(8.0); t.yearlyMaintenanceCost = Money(20.0);
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.6;
        t.groupImpact[SocialGroupType::WORKERS] = -0.3;
        t.groupImpact[SocialGroupType::YOUTH] = -0.5;
        t.groupImpact[SocialGroupType::ACTIVISTS] = -0.8;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Policiamento Comunitário";
        t.description = "Implementa modelo de polícia comunitária com foco em prevenção, presença local e mediação de conflitos. Câmeras corporais obrigatórias, treinamento em direitos humanos de 400h, e ouvidorias externas. Reduz violência policial em 60%.";
        t.category = LawCategory::PUBLIC_SECURITY;
        t.shortTermEffects.criminalRate = -0.01; t.shortTermEffects.happiness = 0.02;
        t.shortTermEffects.governmentSpending = 0.02;
        t.longTermEffects.criminalRate = -0.04; t.longTermEffects.happiness = 0.04;
        t.longTermEffects.stability = 0.03;
        t.implementationDays = 180; t.publicSupport = 0.60; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(10.0); t.yearlyMaintenanceCost = Money(15.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.6;
        t.groupImpact[SocialGroupType::MINORITIES] = 0.5;
        t.groupImpact[SocialGroupType::YOUTH] = 0.4;
        lawTemplates_.push_back(t);
    }

    // ==================== DIREITOS CIVIS (CIVIL_RIGHTS) ====================
    {
        Law t; t.name = "Liberdade de Imprensa e Expressão";
        t.description = "Garante proteção constitucional à liberdade de imprensa e proíbe censura governamental. Pilar democrático fundamental.";
        t.category = LawCategory::CIVIL_RIGHTS;
        t.shortTermEffects.freedom = 0.08; t.shortTermEffects.corruption = -0.02;
        t.shortTermEffects.stability = -0.01;
        t.longTermEffects.freedom = 0.12; t.longTermEffects.corruption = -0.05;
        t.longTermEffects.happiness = 0.03;
        t.implementationDays = 30; t.publicSupport = 0.65; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(0.5);
        t.groupImpact[SocialGroupType::JOURNALISTS] = 0.9;
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.7;
        t.groupImpact[SocialGroupType::NATIONALISTS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Lei de Igualdade de Gênero";
        t.description = "Equiparação salarial obrigatória (multas para diferenças >5%), cotas de 40% em cargos públicos e diretivos, canal de denúncia para assédio com sigilo. Reduz desigualdade estrutural e aumenta produtividade feminina.";
        t.category = LawCategory::CIVIL_RIGHTS;
        t.shortTermEffects.happiness = 0.02; t.shortTermEffects.inequality = -0.02;
        t.shortTermEffects.freedom = 0.02;
        t.longTermEffects.inequality = -0.04; t.longTermEffects.happiness = 0.04;
        t.longTermEffects.gdpGrowth = 0.01;
        t.implementationDays = 180; t.publicSupport = 0.55; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(2.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::WOMEN] = 0.8;
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.6;
        t.groupImpact[SocialGroupType::RELIGIOUS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Estado de Exceção / Lei Marcial";
        t.description = "Suspende garantias constitucionais temporariamente: toque de recolher, censura prévia, prisão sem mandado, proibição de manifestações. Controle total mas destruição democrática e sanções internacionais. Uso extremo apenas.";
        t.category = LawCategory::CIVIL_RIGHTS;
        t.shortTermEffects.freedom = -0.15; t.shortTermEffects.stability = 0.10;
        t.shortTermEffects.criminalRate = -0.08; t.shortTermEffects.happiness = -0.10;
        t.longTermEffects.freedom = -0.20; t.longTermEffects.stability = -0.05;
        t.longTermEffects.corruption = 0.05;
        t.implementationDays = 1; t.publicSupport = 0.10; t.parliamentarySupport = 0.15;
        t.implementationCost = Money(2.0); t.yearlyMaintenanceCost = Money(25.0);
        t.groupImpact[SocialGroupType::MILITARY] = 0.6;
        t.groupImpact[SocialGroupType::ACTIVISTS] = -0.9;
        t.groupImpact[SocialGroupType::JOURNALISTS] = -0.8;
        t.groupImpact[SocialGroupType::WORKERS] = -0.5;
        t.groupImpact[SocialGroupType::STUDENTS] = -0.7;
        lawTemplates_.push_back(t);
    }

    // ==================== SAÚDE (HEALTHCARE) ====================
    {
        Law t; t.name = "Sistema Universal de Saúde";
        t.description = "Cobertura médica universal gratuita financiada por impostos. Melhora drasticamente indicadores de saúde mas tem custo fiscal alto.";
        t.category = LawCategory::HEALTHCARE;
        t.shortTermEffects.governmentSpending = 0.06; t.shortTermEffects.healthcare = 0.03;
        t.shortTermEffects.happiness = 0.03;
        t.longTermEffects.healthcare = 0.10; t.longTermEffects.happiness = 0.06;
        t.longTermEffects.gdpGrowth = 0.01;
        t.implementationDays = 365*2; t.publicSupport = 0.75; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(50.0); t.yearlyMaintenanceCost = Money(120.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.9;
        t.groupImpact[SocialGroupType::ELDERLY] = 0.7;
        t.groupImpact[SocialGroupType::WORKERS] = 0.5;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Privatização do Sistema de Saúde";
        t.description = "Transfere gestão hospitalar ao setor privado via PPPs. Reduz gasto público em 30% mas cria saúde de dois níveis: premium para quem pode pagar e precariária para o resto.";
        t.category = LawCategory::HEALTHCARE;
        t.shortTermEffects.governmentSpending = -0.04; t.shortTermEffects.healthcare = -0.01;
        t.shortTermEffects.inequality = 0.03; t.shortTermEffects.happiness = -0.03;
        t.longTermEffects.governmentSpending = -0.06; t.longTermEffects.inequality = 0.04;
        t.implementationDays = 270; t.publicSupport = 0.25; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(3.0);
        t.groupImpact[SocialGroupType::UPPER_CLASS] = 0.4;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.5;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = -0.7;
        t.groupImpact[SocialGroupType::ELDERLY] = -0.5;
        t.groupImpact[SocialGroupType::PUBLIC_SERVANTS] = -0.6;
        lawTemplates_.push_back(t);
    }

    // ==================== COMÉRCIO (TRADE) ====================
    {
        Law t; t.name = "Acordo de Livre Comércio Amplo";
        t.description = "Eliminação de tarifas com múltiplos parceiros comerciais. Aumenta competitividade e variedade de produtos mas expõe indústria nacional.";
        t.category = LawCategory::TRADE;
        t.shortTermEffects.gdpGrowth = 0.02; t.shortTermEffects.taxRevenue = -0.02;
        t.shortTermEffects.unemployment = 0.01;
        t.longTermEffects.gdpGrowth = 0.04; t.longTermEffects.inflation = -0.01;
        t.longTermEffects.unemployment = -0.01;
        t.implementationDays = 270; t.publicSupport = 0.40; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(3.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.5;
        t.groupImpact[SocialGroupType::WORKERS] = -0.2;
        t.groupImpact[SocialGroupType::FARMERS] = 0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Protecionismo Industrial";
        t.description = "Tarifas de 30-70% sobre importações para proteger indústria nacional. Preserva 2M de empregos mas aumenta preços ao consumidor em 15-25% e gera retaliações comerciais de parceiros.";
        t.category = LawCategory::TRADE;
        t.shortTermEffects.gdpGrowth = -0.01; t.shortTermEffects.taxRevenue = 0.03;
        t.shortTermEffects.inflation = 0.02; t.shortTermEffects.unemployment = -0.01;
        t.longTermEffects.gdpGrowth = -0.02; t.longTermEffects.inflation = 0.03;
        t.implementationDays = 90; t.publicSupport = 0.50; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(1.0);
        t.groupImpact[SocialGroupType::WORKERS] = 0.4;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.3;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = -0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== MILITAR (MILITARY) ====================
    {
        Law t; t.name = "Serviço Militar Obrigatório";
        t.description = "Conscrição de todos os cidadãos aos 18 anos por 12 meses. Aumenta reservas militares mas é impopular e afeta produtividade.";
        t.category = LawCategory::MILITARY;
        t.shortTermEffects.militaryPower = 0.05; t.shortTermEffects.happiness = -0.04;
        t.shortTermEffects.gdpGrowth = -0.01; t.shortTermEffects.governmentSpending = 0.03;
        t.longTermEffects.militaryPower = 0.08; t.longTermEffects.stability = 0.02;
        t.implementationDays = 180; t.publicSupport = 0.25; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(30.0);
        t.groupImpact[SocialGroupType::YOUTH] = -0.6;
        t.groupImpact[SocialGroupType::MILITARY] = 0.5;
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.4;
        t.groupImpact[SocialGroupType::WORKERS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Programa Nuclear Militar";
        t.description = "Desenvolvimento de armas nucleares com capacidade de 20-50 ogivas em 10 anos. Dissuasão máxima mas provoca sanções internacionais devastadoras (perda de 30% do comércio exterior) e isolamento diplomático severo. Custo estimado de R$500B total.";
        t.category = LawCategory::MILITARY;
        t.shortTermEffects.militaryPower = 0.03; t.shortTermEffects.governmentSpending = 0.06;
        t.shortTermEffects.stability = -0.03;
        t.longTermEffects.militaryPower = 0.15; t.longTermEffects.stability = -0.05;
        t.longTermEffects.happiness = -0.03;
        t.implementationDays = 365*5; t.publicSupport = 0.15; t.parliamentarySupport = 0.20;
        t.implementationCost = Money(100.0); t.yearlyMaintenanceCost = Money(50.0);
        t.groupImpact[SocialGroupType::MILITARY] = 0.6;
        t.groupImpact[SocialGroupType::SCIENTISTS] = 0.3;
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = -0.8;
        t.groupImpact[SocialGroupType::ACTIVISTS] = -0.7;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Modernização das Forças Armadas";
        t.description = "Investimento de R$200B em 10 anos: drones autônomos, ciberdefesa com 5000 cybersoldados, satélites milita-res próprios, e treinamento com IA. Melhora eficiência sem aumento de efetivo.";
        t.category = LawCategory::MILITARY;
        t.shortTermEffects.militaryPower = 0.03; t.shortTermEffects.governmentSpending = 0.04;
        t.longTermEffects.militaryPower = 0.08; t.longTermEffects.governmentSpending = 0.02;
        t.implementationDays = 365*2; t.publicSupport = 0.50; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(50.0); t.yearlyMaintenanceCost = Money(20.0);
        t.groupImpact[SocialGroupType::MILITARY] = 0.8;
        t.groupImpact[SocialGroupType::SCIENTISTS] = 0.4;
        t.groupImpact[SocialGroupType::WORKERS] = 0.2;
        lawTemplates_.push_back(t);
    }

    // ==================== MÍDIA (MEDIA) ====================
    {
        Law t; t.name = "Regulamentação de Mídias Sociais";
        t.description = "Obriga remoção de desinformação, transparência algorítmica e proteção de dados. Combate fake news mas levanta questões de censura.";
        t.category = LawCategory::MEDIA;
        t.shortTermEffects.freedom = -0.02; t.shortTermEffects.stability = 0.02;
        t.shortTermEffects.corruption = -0.01;
        t.longTermEffects.stability = 0.04; t.longTermEffects.freedom = -0.01;
        t.longTermEffects.corruption = -0.02;
        t.implementationDays = 180; t.publicSupport = 0.50; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(3.0); t.yearlyMaintenanceCost = Money(2.0);
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.5;
        t.groupImpact[SocialGroupType::JOURNALISTS] = -0.3;
        t.groupImpact[SocialGroupType::YOUTH] = 0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Censura Estatal de Mídia";
        t.description = "Controle governamental sobre meios de comunicação: aprovação prévia de conteúdo, bloqueio de redes sociais, prisão de jornalistas críticos. Suprime oposição mas destrói liberdade de expressão e confiança internacional.";
        t.category = LawCategory::MEDIA;
        t.shortTermEffects.freedom = -0.12; t.shortTermEffects.stability = 0.06;
        t.shortTermEffects.happiness = -0.05; t.shortTermEffects.corruption = 0.03;
        t.longTermEffects.freedom = -0.15; t.longTermEffects.corruption = 0.06;
        t.longTermEffects.stability = -0.02;
        t.implementationDays = 30; t.publicSupport = 0.10; t.parliamentarySupport = 0.20;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(10.0);
        t.groupImpact[SocialGroupType::JOURNALISTS] = -0.9;
        t.groupImpact[SocialGroupType::ACTIVISTS] = -0.8;
        t.groupImpact[SocialGroupType::STUDENTS] = -0.6;
        t.groupImpact[SocialGroupType::MILITARY] = 0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== TECNOLOGIA (TECHNOLOGY) ====================
    {
        Law t; t.name = "Programa Nacional de IA e Tecnologia";
        t.description = "Investimento massivo em pesquisa de IA, biotecnologia, energia quântica. Posiciona o país na vanguarda tecnológica global.";
        t.category = LawCategory::TECHNOLOGY;
        t.shortTermEffects.governmentSpending = 0.04; t.shortTermEffects.education = 0.01;
        t.longTermEffects.gdpGrowth = 0.04; t.longTermEffects.education = 0.03;
        t.longTermEffects.militaryPower = 0.02;
        t.implementationDays = 365*3; t.publicSupport = 0.60; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(40.0); t.yearlyMaintenanceCost = Money(15.0);
        t.groupImpact[SocialGroupType::SCIENTISTS] = 0.9;
        t.groupImpact[SocialGroupType::STUDENTS] = 0.5;
        t.groupImpact[SocialGroupType::MILITARY] = 0.3;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Lei de Proteção de Dados (LGPD/GDPR)";
        t.description = "Regulamenta coleta, armazenamento e uso de dados pessoais. Protege privacidade mas impõe custos de conformidade às empresas.";
        t.category = LawCategory::TECHNOLOGY;
        t.shortTermEffects.freedom = 0.03; t.shortTermEffects.gdpGrowth = -0.005;
        t.shortTermEffects.happiness = 0.02;
        t.longTermEffects.freedom = 0.04; t.longTermEffects.corruption = -0.02;
        t.longTermEffects.gdpGrowth = 0.005;
        t.implementationDays = 180; t.publicSupport = 0.55; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(2.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.3;
        t.groupImpact[SocialGroupType::YOUTH] = 0.4;
        lawTemplates_.push_back(t);
    }

    // ==================== HABITAÇÃO (HOUSING) ====================
    {
        Law t; t.name = "Programa Habitacional Popular";
        t.description = "Construção subsidiada de moradias populares e financiamento facilitado. Reduz déficit habitacional e gera empregos na construção.";
        t.category = LawCategory::HOUSING;
        t.shortTermEffects.governmentSpending = 0.04; t.shortTermEffects.happiness = 0.03;
        t.shortTermEffects.unemployment = -0.01;
        t.longTermEffects.happiness = 0.05; t.longTermEffects.inequality = -0.02;
        t.longTermEffects.stability = 0.02;
        t.implementationDays = 365; t.publicSupport = 0.70; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(25.0); t.yearlyMaintenanceCost = Money(15.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.8;
        t.groupImpact[SocialGroupType::WORKERS] = 0.5;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Imposto sobre Imóveis Vazios";
        t.description = "Taxa progressiva sobre imóveis ociosos em áreas urbanas. Combate especulação imobiliária e amplia oferta de moradia.";
        t.category = LawCategory::HOUSING;
        t.shortTermEffects.taxRevenue = 0.01; t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.inequality = -0.02; t.longTermEffects.happiness = 0.02;
        t.longTermEffects.taxRevenue = 0.02;
        t.implementationDays = 120; t.publicSupport = 0.55; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(0.5);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.5;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.4;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.2;
        lawTemplates_.push_back(t);
    }

    // ==================== TRANSPORTE (TRANSPORTATION) ====================
    {
        Law t; t.name = "Plano Nacional de Transporte Público";
        t.description = "Integração de metrô, ônibus e trens com tarifa subsidiada. Reduz emissões, melhora mobilidade e qualidade de vida.";
        t.category = LawCategory::TRANSPORTATION;
        t.shortTermEffects.governmentSpending = 0.05; t.shortTermEffects.pollution = -0.02;
        t.shortTermEffects.happiness = 0.02;
        t.longTermEffects.pollution = -0.05; t.longTermEffects.gdpGrowth = 0.02;
        t.longTermEffects.happiness = 0.04;
        t.implementationDays = 365*3; t.publicSupport = 0.70; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(80.0); t.yearlyMaintenanceCost = Money(25.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.7;
        t.groupImpact[SocialGroupType::WORKERS] = 0.5;
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = 0.6;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Concessão de Rodovias ao Setor Privado";
        t.description = "Concede rodovias para operação privada com pedágio. Melhora infraestrutura sem gasto público mas aumenta custos para usuários.";
        t.category = LawCategory::TRANSPORTATION;
        t.shortTermEffects.governmentSpending = -0.02; t.shortTermEffects.happiness = -0.01;
        t.longTermEffects.gdpGrowth = 0.01; t.longTermEffects.inequality = 0.01;
        t.implementationDays = 180; t.publicSupport = 0.35; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(2.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.5;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = -0.3;
        t.groupImpact[SocialGroupType::WORKERS] = -0.2;
        lawTemplates_.push_back(t);
    }

    // ==================== ENERGIA (ENERGY) ====================
    {
        Law t; t.name = "Transição Energética Verde";
        t.description = "Meta de 100% energia renovável até 2050. Subsídios para solar/eólica, fim gradual de usinas fósseis. Investimento pesado mas futuro sustentável.";
        t.category = LawCategory::ENERGY;
        t.shortTermEffects.governmentSpending = 0.05; t.shortTermEffects.pollution = -0.03;
        t.shortTermEffects.gdpGrowth = -0.01;
        t.longTermEffects.pollution = -0.12; t.longTermEffects.gdpGrowth = 0.02;
        t.longTermEffects.healthcare = 0.02;
        t.implementationDays = 365*5; t.publicSupport = 0.55; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(60.0); t.yearlyMaintenanceCost = Money(20.0);
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = 0.9;
        t.groupImpact[SocialGroupType::SCIENTISTS] = 0.5;
        t.groupImpact[SocialGroupType::WORKERS] = 0.3;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Programa Nuclear Civil";
        t.description = "Construção de novas usinas nucleares para energia limpa. Alta produção com baixa emissão mas questões de segurança e resíduos.";
        t.category = LawCategory::ENERGY;
        t.shortTermEffects.governmentSpending = 0.04; t.shortTermEffects.pollution = -0.02;
        t.longTermEffects.pollution = -0.06; t.longTermEffects.gdpGrowth = 0.01;
        t.longTermEffects.stability = -0.01;
        t.implementationDays = 365*4; t.publicSupport = 0.35; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(80.0); t.yearlyMaintenanceCost = Money(10.0);
        t.groupImpact[SocialGroupType::SCIENTISTS] = 0.6;
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = -0.5;
        t.groupImpact[SocialGroupType::WORKERS] = 0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== AGRICULTURA (AGRICULTURE) ====================
    {
        Law t; t.name = "Reforma Agrária Ampla";
        t.description = "Redistribuição de terras improdutivas para agricultura familiar. Reduz concentração fundiária mas gera conflitos rurais.";
        t.category = LawCategory::AGRICULTURE;
        t.shortTermEffects.stability = -0.03; t.shortTermEffects.inequality = -0.03;
        t.shortTermEffects.happiness = 0.02;
        t.longTermEffects.inequality = -0.05; t.longTermEffects.gdpGrowth = 0.01;
        t.longTermEffects.stability = 0.01;
        t.implementationDays = 365*2; t.publicSupport = 0.50; t.parliamentarySupport = 0.30;
        t.implementationCost = Money(20.0); t.yearlyMaintenanceCost = Money(8.0);
        t.groupImpact[SocialGroupType::FARMERS] = -0.5;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.7;
        t.groupImpact[SocialGroupType::INDIGENOUS] = 0.5;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.6;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Subsídios ao Agronegócio";
        t.description = "Crédito rural subsidiado e isenções fiscais para grandes produtores. Aumenta exportações mas beneficia desproporcionalmente grandes latifúndios.";
        t.category = LawCategory::AGRICULTURE;
        t.shortTermEffects.gdpGrowth = 0.02; t.shortTermEffects.governmentSpending = 0.03;
        t.shortTermEffects.inequality = 0.01; t.shortTermEffects.pollution = 0.02;
        t.longTermEffects.gdpGrowth = 0.02; t.longTermEffects.pollution = 0.03;
        t.implementationDays = 90; t.publicSupport = 0.35; t.parliamentarySupport = 0.65;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(25.0);
        t.groupImpact[SocialGroupType::FARMERS] = 0.8;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.5;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = -0.2;
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = -0.5;
        lawTemplates_.push_back(t);
    }

    // ==================== RELAÇÕES EXTERIORES (FOREIGN_AFFAIRS) ====================
    {
        Law t; t.name = "Política de Não-Alinhamento";
        t.description = "Posição neutra em conflitos globais, equidistância entre blocos de poder. Mantém soberania mas limita acordos estratégicos.";
        t.category = LawCategory::FOREIGN_AFFAIRS;
        t.shortTermEffects.stability = 0.02; t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.stability = 0.03; t.longTermEffects.gdpGrowth = -0.005;
        t.implementationDays = 30; t.publicSupport = 0.50; t.parliamentarySupport = 0.50;
        t.groupImpact[SocialGroupType::NATIONALISTS] = -0.3;
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Integração Regional";
        t.description = "Aprofundamento de blocos econômicos regionais com livre circulação de pessoas e harmonização regulatória. Fortalece posição regional.";
        t.category = LawCategory::FOREIGN_AFFAIRS;
        t.shortTermEffects.gdpGrowth = 0.01; t.shortTermEffects.immigration = 0.02;
        t.longTermEffects.gdpGrowth = 0.03; t.longTermEffects.stability = 0.02;
        t.longTermEffects.happiness = 0.02;
        t.implementationDays = 365; t.publicSupport = 0.45; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(2.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.4;
        t.groupImpact[SocialGroupType::IMMIGRANTS] = 0.5;
        t.groupImpact[SocialGroupType::NATIONALISTS] = -0.5;
        lawTemplates_.push_back(t);
    }

    // ==================== EXTRAS MULTI-CATEGORIA ====================
    {
        Law t; t.name = "Lei Anticorrupção Rigorosa";
        t.description = "Fichas-limpas, transparência obrigatória, delação premiada e penas severas para corrupção. Limpa política mas gera resistência parlamentar.";
        t.category = LawCategory::CRIMINAL;
        t.shortTermEffects.corruption = -0.05; t.shortTermEffects.stability = -0.02;
        t.shortTermEffects.happiness = 0.03;
        t.longTermEffects.corruption = -0.10; t.longTermEffects.gdpGrowth = 0.01;
        t.longTermEffects.stability = 0.03;
        t.implementationDays = 150; t.publicSupport = 0.80; t.parliamentarySupport = 0.25;
        t.implementationCost = Money(3.0); t.yearlyMaintenanceCost = Money(2.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.6;
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.8;
        t.groupImpact[SocialGroupType::PUBLIC_SERVANTS] = -0.5;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Nacionalização de Recursos Naturais";
        t.description = "Estatização de petróleo, mineração e recursos hídricos. Controle estatal sobre riquezas nacionais mas afasta investimento estrangeiro.";
        t.category = LawCategory::ECONOMIC;
        t.shortTermEffects.gdpGrowth = -0.02; t.shortTermEffects.taxRevenue = 0.06;
        t.shortTermEffects.stability = -0.02;
        t.longTermEffects.taxRevenue = 0.04; t.longTermEffects.gdpGrowth = -0.01;
        t.longTermEffects.corruption = 0.02;
        t.implementationDays = 270; t.publicSupport = 0.55; t.parliamentarySupport = 0.35;
        t.implementationCost = Money(10.0); t.yearlyMaintenanceCost = Money(5.0);
        t.groupImpact[SocialGroupType::WORKERS] = 0.4;
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.7;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.5;
        lawTemplates_.push_back(t);
    }

    // ==================== TRIBUTÁRIA — extras ====================
    {
        Law t; t.name = "Imposto sobre Transações Financeiras";
        t.description = "Taxa de 0.1% sobre cada transação financeira (Tobin tax). Desincentiva especulação de curto prazo, arrecada R$60B/ano e reduz volatilidade cambial, mas eleva custos bancários para famílias.";
        t.category = LawCategory::TAX;
        t.shortTermEffects.taxRevenue = 0.03; t.shortTermEffects.gdpGrowth = -0.005;
        t.shortTermEffects.inequality = -0.01;
        t.longTermEffects.taxRevenue = 0.04; t.longTermEffects.gdpGrowth = -0.01;
        t.implementationDays = 60; t.publicSupport = 0.50; t.parliamentarySupport = 0.35;
        t.implementationCost = Money(1.0);
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.4;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.3;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Imposto Negativo de Renda";
        t.description = "Crédito fiscal para trabalhadores de baixa renda: quem ganha menos da linha de pobreza recebe devolução de até 50% da diferença. Complementa mercado de trabalho sem distorções de salário mínimo.";
        t.category = LawCategory::TAX;
        t.shortTermEffects.inequality = -0.03; t.shortTermEffects.happiness = 0.03;
        t.shortTermEffects.governmentSpending = 0.03;
        t.longTermEffects.inequality = -0.05; t.longTermEffects.happiness = 0.04;
        t.longTermEffects.unemployment = -0.01;
        t.implementationDays = 90; t.publicSupport = 0.55; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(2.0); t.yearlyMaintenanceCost = Money(30.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.8;
        t.groupImpact[SocialGroupType::UNEMPLOYED] = 0.5;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Isenção Tributária para Exportações";
        t.description = "Desoneração total de impostos incidentes sobre produtos exportados. Torna produtos nacionais mais competitivos no mercado global, aumenta reservas cambiais mas reduz arrecadação interna.";
        t.category = LawCategory::TAX;
        t.shortTermEffects.taxRevenue = -0.02; t.shortTermEffects.gdpGrowth = 0.02;
        t.longTermEffects.gdpGrowth = 0.03; t.longTermEffects.taxRevenue = 0.01;
        t.implementationDays = 60; t.publicSupport = 0.55; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(1.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.7;
        t.groupImpact[SocialGroupType::FARMERS] = 0.5;
        t.groupImpact[SocialGroupType::WORKERS] = 0.2;
        lawTemplates_.push_back(t);
    }

    // ==================== ECONÔMICA — extras ====================
    {
        Law t; t.name = "Controle de Capitais";
        t.description = "Restringe fluxo de US$10M+ sem aprovação do Banco Central. Protege contra fuga de capitais e ataques especulativos, mas reduz credibilidade internacional e investimento estrangeiro de longo prazo.";
        t.category = LawCategory::ECONOMIC;
        t.shortTermEffects.gdpGrowth = -0.01; t.shortTermEffects.stability = 0.02;
        t.longTermEffects.gdpGrowth = -0.02; t.longTermEffects.stability = 0.01;
        t.implementationDays = 30; t.publicSupport = 0.40; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(0.5);
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.4;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.5;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Programa de Infraestrutura Nacional";
        t.description = "Investimento de R$500B em 10 anos: 15.000km de ferrovias, 50 aeroportos regionais, portos de última geração e rede de fibra óptica nacional. Maior obra da história do país.";
        t.category = LawCategory::ECONOMIC;
        t.shortTermEffects.gdpGrowth = 0.03; t.shortTermEffects.unemployment = -0.03;
        t.shortTermEffects.governmentSpending = 0.06; t.shortTermEffects.corruption = 0.01;
        t.longTermEffects.gdpGrowth = 0.05; t.longTermEffects.unemployment = -0.02;
        t.longTermEffects.corruption = -0.01;
        t.implementationDays = 365*3; t.publicSupport = 0.70; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(100.0); t.yearlyMaintenanceCost = Money(50.0);
        t.groupImpact[SocialGroupType::WORKERS] = 0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.5;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Abertura Financeira e Câmbio Livre";
        t.description = "Eliminação de restrições cambiais, plena conversibilidade da moeda e livre entrada/saída de capitais. Atrai investimento estrangeiro e moderniza o sistema financeiro, mas expõe a economia a choques externos.";
        t.category = LawCategory::ECONOMIC;
        t.shortTermEffects.gdpGrowth = 0.015; t.shortTermEffects.stability = -0.01;
        t.longTermEffects.gdpGrowth = 0.02; t.longTermEffects.inequality = 0.01;
        t.implementationDays = 90; t.publicSupport = 0.35; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(1.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.6;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = 0.5;
        t.groupImpact[SocialGroupType::WORKERS] = -0.2;
        t.groupImpact[SocialGroupType::NATIONALISTS] = -0.4;
        lawTemplates_.push_back(t);
    }

    // ==================== TRABALHISTA — extras ====================
    {
        Law t; t.name = "Regularização do Trabalho Informal";
        t.description = "Programa de transição da informalidade: empresas com até 5 funcionários pagam 50% das contribuições previdenciárias por 3 anos. Inclui assistência técnica gratuita. Formaliza 8M de trabalhadores estima-se.";
        t.category = LawCategory::LABOR;
        t.shortTermEffects.taxRevenue = 0.02; t.shortTermEffects.unemployment = -0.01;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.taxRevenue = 0.04; t.longTermEffects.inequality = -0.02;
        t.longTermEffects.unemployment = -0.02;
        t.implementationDays = 180; t.publicSupport = 0.65; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(3.0);
        t.groupImpact[SocialGroupType::WORKERS] = 0.6;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.5;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Proibição de Trabalho Infantil";
        t.description = "Proibição total de trabalho de menores de 16 anos (exceto aprendizes 14+), fiscalização intensiva com multas de R$5M por infração e erradicação do trabalho escravo rural. Protege crianças e aumenta frequência escolar.";
        t.category = LawCategory::LABOR;
        t.shortTermEffects.education = 0.02; t.shortTermEffects.happiness = 0.02;
        t.shortTermEffects.gdpGrowth = -0.005;
        t.longTermEffects.education = 0.04; t.longTermEffects.happiness = 0.03;
        t.longTermEffects.gdpGrowth = 0.01;
        t.implementationDays = 90; t.publicSupport = 0.80; t.parliamentarySupport = 0.70;
        t.implementationCost = Money(2.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::STUDENTS] = 0.7;
        t.groupImpact[SocialGroupType::YOUTH] = 0.6;
        t.groupImpact[SocialGroupType::FARMERS] = -0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Participação Obrigatória nos Lucros";
        t.description = "Empresas com >50 funcionários devem distribuir 10% do lucro líquido anual. Modelo de codeterminação: 2 assentos no conselho para representantes dos trabalhadores. Reduz desigualdade e aumenta produtividade mas eleva resistência empresarial.";
        t.category = LawCategory::LABOR;
        t.shortTermEffects.happiness = 0.03; t.shortTermEffects.inequality = -0.02;
        t.shortTermEffects.gdpGrowth = -0.005;
        t.longTermEffects.happiness = 0.04; t.longTermEffects.inequality = -0.03;
        t.longTermEffects.gdpGrowth = 0.01;
        t.implementationDays = 150; t.publicSupport = 0.65; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(1.0);
        t.groupImpact[SocialGroupType::WORKERS] = 0.8;
        t.groupImpact[SocialGroupType::UNIONS] = 0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.5;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.4;
        lawTemplates_.push_back(t);
    }

    // ==================== PREVIDÊNCIA — extras ====================
    {
        Law t; t.name = "Fundo Soberano Intergeracional";
        t.description = "Cria fundo com 5% da arrecadação anual de royalties de petróleo e mineração para custeio futuro da previdência. Modelo norueguês. Horizon de 30 anos para impacto real mas protege gerações futuras.";
        t.category = LawCategory::PENSION;
        t.shortTermEffects.governmentSpending = 0.02; t.shortTermEffects.stability = 0.01;
        t.longTermEffects.governmentSpending = -0.04; t.longTermEffects.stability = 0.03;
        t.longTermEffects.happiness = 0.01;
        t.implementationDays = 365; t.publicSupport = 0.60; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(10.0); t.yearlyMaintenanceCost = Money(3.0);
        t.groupImpact[SocialGroupType::YOUTH] = 0.6;
        t.groupImpact[SocialGroupType::STUDENTS] = 0.5;
        t.groupImpact[SocialGroupType::RETIREES] = 0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Aposentadoria Antecipada para Funções Insalubres";
        t.description = "Mineradores, bombeiros, agricultores e trabalhadores em condições insalubres se aposentam 10 anos antes. Reconhece desgaste diferenciado. Custo adicional de R$40B/ano mas melhora condições de categorias vulneráveis.";
        t.category = LawCategory::PENSION;
        t.shortTermEffects.happiness = 0.03; t.shortTermEffects.governmentSpending = 0.02;
        t.shortTermEffects.stability = 0.01;
        t.longTermEffects.happiness = 0.04; t.longTermEffects.governmentSpending = 0.03;
        t.implementationDays = 180; t.publicSupport = 0.60; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(3.0); t.yearlyMaintenanceCost = Money(40.0);
        t.groupImpact[SocialGroupType::WORKERS] = 0.7;
        t.groupImpact[SocialGroupType::FARMERS] = 0.6;
        t.groupImpact[SocialGroupType::UNIONS] = 0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== AMBIENTAL — extras ====================
    {
        Law t; t.name = "Reflorestamento Nacional";
        t.description = "Meta de plantar 2 bilhões de árvores em 10 anos. Recupera biomas degradados, gera 500 mil empregos verdes e compensa emissões de CO2 equivalente a 15% das emissões anuais. Modelo de pagamento por resultado.";
        t.category = LawCategory::ENVIRONMENTAL;
        t.shortTermEffects.unemployment = -0.01; t.shortTermEffects.pollution = -0.01;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.pollution = -0.04; t.longTermEffects.happiness = 0.02;
        t.longTermEffects.healthcare = 0.01;
        t.implementationDays = 365*2; t.publicSupport = 0.70; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(15.0); t.yearlyMaintenanceCost = Money(5.0);
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = 0.8;
        t.groupImpact[SocialGroupType::INDIGENOUS] = 0.6;
        t.groupImpact[SocialGroupType::FARMERS] = 0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Proibição de Plásticos Descartáveis";
        t.description = "Banimento de sacolinhas, canudos, copinhos e embalagens plásticas de uso único. Prazo de transição de 2 anos. Reduz poluição oceânica mas impõe custos à indústria de embalagens. Incentivo de R$5B para transição para biodegradáveis.";
        t.category = LawCategory::ENVIRONMENTAL;
        t.shortTermEffects.pollution = -0.02; t.shortTermEffects.gdpGrowth = -0.005;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.pollution = -0.04; t.longTermEffects.healthcare = 0.01;
        t.implementationDays = 180; t.publicSupport = 0.60; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = 0.7;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.3;
        t.groupImpact[SocialGroupType::YOUTH] = 0.5;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Taxa de Carbono nas Importações";
        t.description = "Ajuste de carbono nas fronteiras: importações de países sem precificação de carbono pagam taxa de US$50/tonCO2 embutida. Equaliza competição, arrecada US$20B/ano mas provoca retaliações comerciais.";
        t.category = LawCategory::ENVIRONMENTAL;
        t.shortTermEffects.taxRevenue = 0.01; t.shortTermEffects.gdpGrowth = -0.01;
        t.shortTermEffects.pollution = -0.01;
        t.longTermEffects.taxRevenue = 0.02; t.longTermEffects.pollution = -0.03;
        t.implementationDays = 180; t.publicSupport = 0.45; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(2.0);
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = 0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.2;
        t.groupImpact[SocialGroupType::FARMERS] = -0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== EDUCAÇÃO — extras ====================
    {
        Law t; t.name = "Cotas para Grupos Sub-representados";
        t.description = "30% das vagas em universidades federais e concursos públicos reservadas para autodeclarados negros, indígenas e pessoas de baixa renda. Ação afirmativa histórica que reduz desigualdade estrutural mas gera debate sobre mérito.";
        t.category = LawCategory::EDUCATION;
        t.shortTermEffects.inequality = -0.02; t.shortTermEffects.happiness = 0.01;
        t.shortTermEffects.stability = -0.01;
        t.longTermEffects.inequality = -0.04; t.longTermEffects.happiness = 0.02;
        t.longTermEffects.education = 0.02;
        t.implementationDays = 90; t.publicSupport = 0.45; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(1.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.7;
        t.groupImpact[SocialGroupType::MINORITIES] = 0.8;
        t.groupImpact[SocialGroupType::INDIGENOUS] = 0.8;
        t.groupImpact[SocialGroupType::UPPER_MIDDLE_CLASS] = -0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Escola de Tempo Integral";
        t.description = "Expansão para 10h diárias nas escolas públicas: turno normal + reforço, esporte, artes e alimentação. Cobre 100% das crianças até 2035. Reduz evasão e violência, melhora aprendizado, mas exige 400.000 novos professores.";
        t.category = LawCategory::EDUCATION;
        t.shortTermEffects.governmentSpending = 0.05; t.shortTermEffects.education = 0.02;
        t.shortTermEffects.happiness = 0.02;
        t.longTermEffects.education = 0.07; t.longTermEffects.criminalRate = -0.02;
        t.longTermEffects.happiness = 0.03;
        t.implementationDays = 365*3; t.publicSupport = 0.75; t.parliamentarySupport = 0.65;
        t.implementationCost = Money(30.0); t.yearlyMaintenanceCost = Money(60.0);
        t.groupImpact[SocialGroupType::STUDENTS] = 0.7;
        t.groupImpact[SocialGroupType::TEACHERS] = 0.8;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.6;
        t.groupImpact[SocialGroupType::WOMEN] = 0.5;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Ensino Profissionalizante Gratuito";
        t.description = "Rede nacional de escolas técnicas gratuitas em parceria com CNI/SENAI. 2 milhões de vagas/ano em tecnologia, saúde, energia e serviços. Qualifica força de trabalho e reduz gap de habilidades da indústria.";
        t.category = LawCategory::EDUCATION;
        t.shortTermEffects.governmentSpending = 0.03; t.shortTermEffects.unemployment = -0.01;
        t.shortTermEffects.education = 0.01;
        t.longTermEffects.unemployment = -0.03; t.longTermEffects.gdpGrowth = 0.02;
        t.longTermEffects.education = 0.04;
        t.implementationDays = 365*2; t.publicSupport = 0.70; t.parliamentarySupport = 0.65;
        t.implementationCost = Money(20.0); t.yearlyMaintenanceCost = Money(30.0);
        t.groupImpact[SocialGroupType::WORKERS] = 0.7;
        t.groupImpact[SocialGroupType::YOUTH] = 0.6;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.5;
        lawTemplates_.push_back(t);
    }

    // ==================== CRIMINAL — extras ====================
    {
        Law t; t.name = "Monitoramento Eletrônico como Alternativa";
        t.description = "Tornozeleiras eletrônicas para condenados por crimes não-violentos com pena até 8 anos. Reduz população carcerária de 900mil para 600mil em 5 anos, economizando R$15B anuais. Recidiva 20% menor que prisão.";
        t.category = LawCategory::CRIMINAL;
        t.shortTermEffects.governmentSpending = -0.01; t.shortTermEffects.freedom = 0.02;
        t.shortTermEffects.criminalRate = 0.01;
        t.longTermEffects.governmentSpending = -0.03; t.longTermEffects.criminalRate = -0.02;
        t.longTermEffects.happiness = 0.01;
        t.implementationDays = 120; t.publicSupport = 0.50; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(4.0); t.yearlyMaintenanceCost = Money(2.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.4;
        t.groupImpact[SocialGroupType::YOUTH] = 0.3;
        t.groupImpact[SocialGroupType::UPPER_MIDDLE_CLASS] = -0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Legalização e Regulamentação da Cannabis";
        t.description = "Legalização para adultos (18+) com venda em estabelecimentos licenciados, imposto de 30% e proibição de publicidade. Mercado estimado em R$45B/ano. Elimina tráfico de maconha, mas cresce consumo 15-25%.";
        t.category = LawCategory::CRIMINAL;
        t.shortTermEffects.taxRevenue = 0.02; t.shortTermEffects.criminalRate = -0.02;
        t.shortTermEffects.happiness = 0.01; t.shortTermEffects.freedom = 0.03;
        t.longTermEffects.taxRevenue = 0.03; t.longTermEffects.criminalRate = -0.04;
        t.longTermEffects.governmentSpending = -0.01;
        t.implementationDays = 150; t.publicSupport = 0.45; t.parliamentarySupport = 0.35;
        t.implementationCost = Money(2.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::YOUTH] = 0.6;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.4;
        t.groupImpact[SocialGroupType::RELIGIOUS] = -0.7;
        t.groupImpact[SocialGroupType::ELDERLY] = -0.4;
        lawTemplates_.push_back(t);
    }

    // ==================== IMIGRAÇÃO — extras ====================
    {
        Law t; t.name = "Programa de Imigração por Pontos";
        t.description = "Sistema australiano/canadense: pontuação por escolaridade, idioma, experiência e área de atuação. Prioriza profissionais qualificados em setores deficitários. Atrai 200mil imigrantes/ano de alta qualificação.";
        t.category = LawCategory::IMMIGRATION;
        t.shortTermEffects.immigration = 0.03; t.shortTermEffects.gdpGrowth = 0.01;
        t.shortTermEffects.taxRevenue = 0.01;
        t.longTermEffects.gdpGrowth = 0.02; t.longTermEffects.immigration = 0.05;
        t.longTermEffects.education = 0.01;
        t.implementationDays = 180; t.publicSupport = 0.50; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(3.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.5;
        t.groupImpact[SocialGroupType::IMMIGRANTS] = 0.6;
        t.groupImpact[SocialGroupType::NATIONALISTS] = -0.4;
        t.groupImpact[SocialGroupType::WORKERS] = -0.1;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Anistia para Imigrantes Irregulares";
        t.description = "Regularização de imigrantes há mais de 3 anos no país: documento, CPF e acesso a serviços públicos. Afeta estimados 2M de pessoas. Formaliza trabalho, aumenta arrecadação mas gera pressão sobre serviços públicos.";
        t.category = LawCategory::IMMIGRATION;
        t.shortTermEffects.immigration = 0.04; t.shortTermEffects.taxRevenue = 0.01;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.taxRevenue = 0.02; t.longTermEffects.inequality = -0.01;
        t.implementationDays = 90; t.publicSupport = 0.35; t.parliamentarySupport = 0.35;
        t.implementationCost = Money(2.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::IMMIGRANTS] = 0.9;
        t.groupImpact[SocialGroupType::WORKERS] = -0.2;
        t.groupImpact[SocialGroupType::NATIONALISTS] = -0.8;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Expulsão de Criminosos Estrangeiros";
        t.description = "Deportação imediata de imigrantes com condenação criminal, revogação automática de visto para irregulares com antecedentes. Lista de países de alto risco com banimento de entrada.";
        t.category = LawCategory::IMMIGRATION;
        t.shortTermEffects.criminalRate = -0.01; t.shortTermEffects.stability = 0.01;
        t.shortTermEffects.freedom = -0.01;
        t.longTermEffects.criminalRate = -0.02; t.longTermEffects.stability = 0.02;
        t.implementationDays = 60; t.publicSupport = 0.55; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(1.0); t.yearlyMaintenanceCost = Money(0.5);
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.6;
        t.groupImpact[SocialGroupType::IMMIGRANTS] = -0.7;
        t.groupImpact[SocialGroupType::ACTIVISTS] = -0.4;
        lawTemplates_.push_back(t);
    }

    // ==================== SEGURANÇA PÚBLICA — extras ====================
    {
        Law t; t.name = "Câmeras de Reconhecimento Facial";
        t.description = "Rede de 500mil câmeras com reconhecimento facial em cidades. Identifica fugitivos em tempo real, reduz crime 15-30%, mas cria Estado de vigilância e aumenta risco de falsos positivos (tasa de erro de 5% em negros).";
        t.category = LawCategory::PUBLIC_SECURITY;
        t.shortTermEffects.criminalRate = -0.03; t.shortTermEffects.freedom = -0.04;
        t.shortTermEffects.happiness = -0.01;
        t.longTermEffects.criminalRate = -0.05; t.longTermEffects.freedom = -0.06;
        t.longTermEffects.stability = 0.02;
        t.implementationDays = 270; t.publicSupport = 0.45; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(20.0); t.yearlyMaintenanceCost = Money(5.0);
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.4;
        t.groupImpact[SocialGroupType::ACTIVISTS] = -0.7;
        t.groupImpact[SocialGroupType::MINORITIES] = -0.5;
        t.groupImpact[SocialGroupType::YOUTH] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Desmilitarização da Polícia";
        t.description = "Proíbe uso de fuzis e viaturas blindadas pela polícia civil, cria unidades de saúde mental para crises, e substitui abordagem bélica por policiamento orientado à solução de problemas. Reduz violência policial mas aumenta risco para agentes.";
        t.category = LawCategory::PUBLIC_SECURITY;
        t.shortTermEffects.freedom = 0.04; t.shortTermEffects.happiness = 0.02;
        t.shortTermEffects.criminalRate = 0.01;
        t.longTermEffects.freedom = 0.05; t.longTermEffects.happiness = 0.03;
        t.longTermEffects.criminalRate = -0.01;
        t.implementationDays = 180; t.publicSupport = 0.50; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(8.0); t.yearlyMaintenanceCost = Money(5.0);
        t.groupImpact[SocialGroupType::MINORITIES] = 0.6;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.5;
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.6;
        t.groupImpact[SocialGroupType::MILITARY] = -0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== DIREITOS CIVIS — extras ====================
    {
        Law t; t.name = "Lei de Proteção de Minorias";
        t.description = "Crimes de ódio com pena em dobro, quotas mínimas em mídia e propaganda, e legislação anti-discriminação em empregos e serviços. Protege LGBTQIA+, negros, indígenas e pessoas com deficiência.";
        t.category = LawCategory::CIVIL_RIGHTS;
        t.shortTermEffects.freedom = 0.04; t.shortTermEffects.happiness = 0.02;
        t.shortTermEffects.stability = -0.01;
        t.longTermEffects.freedom = 0.06; t.longTermEffects.happiness = 0.03;
        t.longTermEffects.inequality = -0.02;
        t.implementationDays = 90; t.publicSupport = 0.50; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(1.0); t.yearlyMaintenanceCost = Money(0.5);
        t.groupImpact[SocialGroupType::MINORITIES] = 0.9;
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.7;
        t.groupImpact[SocialGroupType::RELIGIOUS] = -0.4;
        t.groupImpact[SocialGroupType::NATIONALISTS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Direito ao Voto Eletrônico Digital";
        t.description = "Voto online seguro via blockchain com autenticação biométrica. Aumenta participação eleitoral de 74% para 90%+, inclui idosos e pessoas com mobilidade reduzida. Auditado por entidade independente internacional.";
        t.category = LawCategory::CIVIL_RIGHTS;
        t.shortTermEffects.freedom = 0.03; t.shortTermEffects.stability = 0.01;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.freedom = 0.04; t.longTermEffects.stability = 0.02;
        t.longTermEffects.corruption = -0.02;
        t.implementationDays = 365; t.publicSupport = 0.65; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::YOUTH] = 0.7;
        t.groupImpact[SocialGroupType::ELDERLY] = 0.4;
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.5;
        t.groupImpact[SocialGroupType::NATIONALISTS] = -0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Referendos Obrigatórios para Grandes Obras";
        t.description = "Consulta popular obrigatória para projetos >R$10B, privatizações e mudanças constitucionais. Aprofunda democracia direta mas pode atrasar investimentos urgentes e facilitar populismo.";
        t.category = LawCategory::CIVIL_RIGHTS;
        t.shortTermEffects.freedom = 0.05; t.shortTermEffects.stability = -0.01;
        t.shortTermEffects.gdpGrowth = -0.005;
        t.longTermEffects.freedom = 0.06; t.longTermEffects.corruption = -0.03;
        t.longTermEffects.stability = 0.01;
        t.implementationDays = 60; t.publicSupport = 0.65; t.parliamentarySupport = 0.35;
        t.implementationCost = Money(0.5);
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.8;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.5;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.4;
        t.groupImpact[SocialGroupType::PUBLIC_SERVANTS] = -0.2;
        lawTemplates_.push_back(t);
    }

    // ==================== SAÚDE — extras ====================
    {
        Law t; t.name = "Programa Nacional de Saúde Mental";
        t.description = "Rede de 3000 Centros de Atenção Psicossocial (CAPS), internaão voluntária gratuita e 50mil psicólogos nas escolas e UBS. Reduz suicídio (4o causa de morte em jovens), alcoolismo e uso de drogas.";
        t.category = LawCategory::HEALTHCARE;
        t.shortTermEffects.healthcare = 0.02; t.shortTermEffects.happiness = 0.03;
        t.shortTermEffects.governmentSpending = 0.03;
        t.longTermEffects.healthcare = 0.05; t.longTermEffects.happiness = 0.05;
        t.longTermEffects.criminalRate = -0.02;
        t.implementationDays = 365*2; t.publicSupport = 0.70; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(15.0); t.yearlyMaintenanceCost = Money(20.0);
        t.groupImpact[SocialGroupType::YOUTH] = 0.6;
        t.groupImpact[SocialGroupType::WORKERS] = 0.5;
        t.groupImpact[SocialGroupType::ELDERLY] = 0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Regulamentação de Planos de Saúde";
        t.description = "Proíbe reajustes acima da inflação, limita carências a 6 meses, obriga cobertura integral para doenças preexistentes e estabelece prazos máximos de atendimento. Beneficia 50M de usuários.";
        t.category = LawCategory::HEALTHCARE;
        t.shortTermEffects.happiness = 0.02; t.shortTermEffects.healthcare = 0.01;
        t.shortTermEffects.inequality = -0.02;
        t.longTermEffects.happiness = 0.03; t.longTermEffects.inequality = -0.03;
        t.implementationDays = 90; t.publicSupport = 0.75; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(0.5);
        t.groupImpact[SocialGroupType::UPPER_MIDDLE_CLASS] = 0.6;
        t.groupImpact[SocialGroupType::WORKERS] = 0.4;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Medicina Preventiva Nacional";
        t.description = "Rastreamento universal para câncer, diabetes e hipertensão. Vacinação obrigatória atualizada, check-ups anuais gratuitos e campanhas nacionais de prevenção. Reduz mortalidade prematura em 30% em 10 anos.";
        t.category = LawCategory::HEALTHCARE;
        t.shortTermEffects.governmentSpending = 0.03; t.shortTermEffects.healthcare = 0.02;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.healthcare = 0.08; t.longTermEffects.happiness = 0.04;
        t.longTermEffects.governmentSpending = -0.02;
        t.implementationDays = 365; t.publicSupport = 0.75; t.parliamentarySupport = 0.65;
        t.implementationCost = Money(8.0); t.yearlyMaintenanceCost = Money(15.0);
        t.groupImpact[SocialGroupType::ELDERLY] = 0.7;
        t.groupImpact[SocialGroupType::WOMEN] = 0.5;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.5;
        lawTemplates_.push_back(t);
    }

    // ==================== COMÉRCIO — extras ====================
    {
        Law t; t.name = "Programa de Substituição de Importações";
        t.description = "Política industrial seletiva: crédito subsidiado para setores onde importações >70% da demanda (semicondutores, farmácias, máquinas). Reduz dependência externa em áreas estratégicas em 10 anos.";
        t.category = LawCategory::TRADE;
        t.shortTermEffects.gdpGrowth = -0.005; t.shortTermEffects.governmentSpending = 0.03;
        t.shortTermEffects.unemployment = -0.01;
        t.longTermEffects.gdpGrowth = 0.02; t.longTermEffects.taxRevenue = 0.01;
        t.longTermEffects.unemployment = -0.02;
        t.implementationDays = 365*2; t.publicSupport = 0.50; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(30.0); t.yearlyMaintenanceCost = Money(10.0);
        t.groupImpact[SocialGroupType::WORKERS] = 0.4;
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.5;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Centro Financeiro Internacional";
        t.description = "Criação de hub financeiro offshore com regulação favorável, isenções para fundos internacionais e fuso horário estratégico. Modelo Cingapura/Dubai. Atrai US$500B em ativos mas é criticado por facilitar evasão fiscal global.";
        t.category = LawCategory::TRADE;
        t.shortTermEffects.gdpGrowth = 0.02; t.shortTermEffects.taxRevenue = -0.01;
        t.shortTermEffects.inequality = 0.02;
        t.longTermEffects.gdpGrowth = 0.04; t.longTermEffects.corruption = 0.02;
        t.longTermEffects.inequality = 0.03;
        t.implementationDays = 365*2; t.publicSupport = 0.30; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(20.0); t.yearlyMaintenanceCost = Money(3.0);
        t.groupImpact[SocialGroupType::UPPER_CLASS] = 0.7;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.6;
        t.groupImpact[SocialGroupType::ACTIVISTS] = -0.6;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Embargo Comercial Seletivo";
        t.description = "Suspensão de comércio com países que descumprem padrões de direitos humanos, corrupção ou segurança. Mecanismo de pressão diplomática com impacto econômico bilateral.";
        t.category = LawCategory::TRADE;
        t.shortTermEffects.gdpGrowth = -0.01; t.shortTermEffects.stability = 0.01;
        t.longTermEffects.gdpGrowth = -0.005; t.longTermEffects.stability = 0.02;
        t.implementationDays = 30; t.publicSupport = 0.50; t.parliamentarySupport = 0.55;
        t.implementationCost = Money(0.5);
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.5;
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.4;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.4;
        t.groupImpact[SocialGroupType::FARMERS] = -0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== MILITAR — extras ====================
    {
        Law t; t.name = "Forças de Paz e Operações Internacionais";
        t.description = "Criação de brigada especializada em peacekeeping e operações humanitárias da ONU. 5000 efetivos treinados para zonas de conflito. Aumenta prestígio internacional e soft power, mas envolve risco para militares.";
        t.category = LawCategory::MILITARY;
        t.shortTermEffects.militaryPower = 0.01; t.shortTermEffects.governmentSpending = 0.01;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.militaryPower = 0.03; t.longTermEffects.happiness = 0.02;
        t.longTermEffects.stability = 0.01;
        t.implementationDays = 365; t.publicSupport = 0.55; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(8.0);
        t.groupImpact[SocialGroupType::MILITARY] = 0.6;
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.4;
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.3;
        t.groupImpact[SocialGroupType::WORKERS] = 0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Unidade de Ciberdefesa Nacional";
        t.description = "Criação do Comando Cibernético com 10.000 especialistas, centros de resposta a incidentes e capacidade ofensiva em ataques a infraestrutura inimiga. Protege redes críticas (energia, finanças, comunicações).";
        t.category = LawCategory::MILITARY;
        t.shortTermEffects.militaryPower = 0.03; t.shortTermEffects.governmentSpending = 0.03;
        t.longTermEffects.militaryPower = 0.06; t.longTermEffects.stability = 0.02;
        t.implementationDays = 365*2; t.publicSupport = 0.55; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(15.0); t.yearlyMaintenanceCost = Money(8.0);
        t.groupImpact[SocialGroupType::MILITARY] = 0.7;
        t.groupImpact[SocialGroupType::SCIENTISTS] = 0.5;
        t.groupImpact[SocialGroupType::STUDENTS] = 0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Indústria de Defesa Nacional";
        t.description = "Fomento à produção doméstica de material bélico: tanques, aviões de caça, navios e munição. Reduz dependência de importações militares e gera 200mil empregos de alta qualificação em 10 anos.";
        t.category = LawCategory::MILITARY;
        t.shortTermEffects.governmentSpending = 0.04; t.shortTermEffects.unemployment = -0.01;
        t.shortTermEffects.militaryPower = 0.02;
        t.longTermEffects.militaryPower = 0.07; t.longTermEffects.gdpGrowth = 0.01;
        t.longTermEffects.unemployment = -0.02;
        t.implementationDays = 365*3; t.publicSupport = 0.50; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(50.0); t.yearlyMaintenanceCost = Money(20.0);
        t.groupImpact[SocialGroupType::MILITARY] = 0.6;
        t.groupImpact[SocialGroupType::WORKERS] = 0.4;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.5;
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = -0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== MÍDIA — extras ====================
    {
        Law t; t.name = "Financiamento Público de Mídia Independente";
        t.description = "Fundo de R$5B para jornalismo local sem fins lucrativos, rádios comunitárias e plataformas cooperativas. Contrabalança concentração midiática privada e garante pluralismo informativo nas regiões periféricas.";
        t.category = LawCategory::MEDIA;
        t.shortTermEffects.freedom = 0.04; t.shortTermEffects.corruption = -0.02;
        t.shortTermEffects.governmentSpending = 0.01;
        t.longTermEffects.freedom = 0.06; t.longTermEffects.corruption = -0.04;
        t.longTermEffects.stability = 0.01;
        t.implementationDays = 180; t.publicSupport = 0.50; t.parliamentarySupport = 0.40;
        t.implementationCost = Money(2.0); t.yearlyMaintenanceCost = Money(5.0);
        t.groupImpact[SocialGroupType::JOURNALISTS] = 0.7;
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.6;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.3;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Proibição de Monopólios de Mídia";
        t.description = "Empresa única não pode controlar mais de 30% dos veículos de comunicação em qualquer plataforma. Obriga desinvestimento das 5 maiores redes de TV, que hoje controlam 90% da audiência.";
        t.category = LawCategory::MEDIA;
        t.shortTermEffects.freedom = 0.05; t.shortTermEffects.stability = -0.01;
        t.shortTermEffects.corruption = -0.01;
        t.longTermEffects.freedom = 0.07; t.longTermEffects.corruption = -0.03;
        t.implementationDays = 270; t.publicSupport = 0.55; t.parliamentarySupport = 0.30;
        t.implementationCost = Money(2.0);
        t.groupImpact[SocialGroupType::JOURNALISTS] = 0.6;
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.5;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.5;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Transparência Algorítmica Obrigatória";
        t.description = "Redes sociais com >10M de usuários devem publicar algoritmos de recomendação, log de moderação e critérios de amplificação. Combate câmaras de eco e manipulação eleitoral. Resistência das big techs esperada.";
        t.category = LawCategory::MEDIA;
        t.shortTermEffects.freedom = 0.03; t.shortTermEffects.stability = 0.01;
        t.shortTermEffects.gdpGrowth = -0.005;
        t.longTermEffects.freedom = 0.04; t.longTermEffects.corruption = -0.02;
        t.implementationDays = 180; t.publicSupport = 0.55; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(1.0); t.yearlyMaintenanceCost = Money(0.5);
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.7;
        t.groupImpact[SocialGroupType::YOUTH] = 0.5;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.5;
        lawTemplates_.push_back(t);
    }

    // ==================== TECNOLOGIA — extras ====================
    {
        Law t; t.name = "Internet de Alta Velocidade Universal";
        t.description = "Fibra óptica e 5G para 100% da população até 2035. Inclusão digital de 30M de excluídos. Custo de R$200B via PPP. Aumenta produtividade, educação à distância e telemedicina em zonas remotas.";
        t.category = LawCategory::TECHNOLOGY;
        t.shortTermEffects.governmentSpending = 0.04; t.shortTermEffects.happiness = 0.02;
        t.shortTermEffects.education = 0.01;
        t.longTermEffects.gdpGrowth = 0.03; t.longTermEffects.education = 0.03;
        t.longTermEffects.inequality = -0.03;
        t.implementationDays = 365*3; t.publicSupport = 0.75; t.parliamentarySupport = 0.65;
        t.implementationCost = Money(50.0); t.yearlyMaintenanceCost = Money(15.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.7;
        t.groupImpact[SocialGroupType::STUDENTS] = 0.6;
        t.groupImpact[SocialGroupType::FARMERS] = 0.5;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Regulamentação de Inteligência Artificial";
        t.description = "Framework nacional para IA: proibição de sistemas de crédito social, auditoria obrigatória de IA em decisões de crédito/saúde/emprego, e responsabilidade civil por danos causados por algoritmos.";
        t.category = LawCategory::TECHNOLOGY;
        t.shortTermEffects.freedom = 0.03; t.shortTermEffects.gdpGrowth = -0.005;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.freedom = 0.04; t.longTermEffects.corruption = -0.02;
        t.longTermEffects.gdpGrowth = 0.01;
        t.implementationDays = 180; t.publicSupport = 0.55; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(3.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.6;
        t.groupImpact[SocialGroupType::SCIENTISTS] = 0.4;
        t.groupImpact[SocialGroupType::WORKERS] = 0.3;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Blockchain para Serviços Públicos";
        t.description = "Utilização de blockchain para registros públicos, contratos governamentais, licitações e votações. Torna processos imutáveis e transparentes, reduzindo corrupção em contratos públicos (estimado R$200B/ano em desvios).";
        t.category = LawCategory::TECHNOLOGY;
        t.shortTermEffects.corruption = -0.03; t.shortTermEffects.governmentSpending = 0.01;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.corruption = -0.06; t.longTermEffects.stability = 0.02;
        t.longTermEffects.gdpGrowth = 0.01;
        t.implementationDays = 365*2; t.publicSupport = 0.50; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(10.0); t.yearlyMaintenanceCost = Money(2.0);
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.5;
        t.groupImpact[SocialGroupType::SCIENTISTS] = 0.6;
        t.groupImpact[SocialGroupType::PUBLIC_SERVANTS] = -0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== HABITAÇÃO — extras ====================
    {
        Law t; t.name = "Reforma do Inquilinato";
        t.description = "Limite de reajuste anual de aluguel em IPCA+2%, agiliza despejo por falta de pagamento para 90 dias (antes 2+ anos), e cria Fundo de Garantia de Locação para substituir fiador. Equilibra direitos de proprietários e inquilinos.";
        t.category = LawCategory::HOUSING;
        t.shortTermEffects.happiness = 0.02; t.shortTermEffects.inequality = -0.01;
        t.shortTermEffects.stability = 0.01;
        t.longTermEffects.happiness = 0.03; t.longTermEffects.inequality = -0.02;
        t.longTermEffects.stability = 0.01;
        t.implementationDays = 90; t.publicSupport = 0.55; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(0.5);
        t.groupImpact[SocialGroupType::WORKERS] = 0.4;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.5;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.3;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Zoneamento Misto e Alta Densidade";
        t.description = "Reforma urbana que permite construção de 20+ andares a 600m de estações de metrô e BRT. Elimina zonas exclusivamente residenciais. Reduz custo habitacional em 20-40% nas capitais e diminui tempo de commute.";
        t.category = LawCategory::HOUSING;
        t.shortTermEffects.happiness = -0.01; t.shortTermEffects.gdpGrowth = 0.01;
        t.shortTermEffects.unemployment = -0.01;
        t.longTermEffects.happiness = 0.02; t.longTermEffects.gdpGrowth = 0.02;
        t.longTermEffects.inequality = -0.02;
        t.implementationDays = 180; t.publicSupport = 0.40; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(3.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.5;
        t.groupImpact[SocialGroupType::WORKERS] = 0.4;
        t.groupImpact[SocialGroupType::UPPER_MIDDLE_CLASS] = -0.3;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.3;
        lawTemplates_.push_back(t);
    }

    // ==================== TRANSPORTE — extras ====================
    {
        Law t; t.name = "Ferrovia de Alta Velocidade";
        t.description = "Trem-bala conectando 10 capitais a 300km/h. Viagem SP-RJ em 1h. R$400B em 15 anos mas gera 200mil empregos, reduz emissões do transporte em 20% e desafoga rodovias congestionadas.";
        t.category = LawCategory::TRANSPORTATION;
        t.shortTermEffects.governmentSpending = 0.06; t.shortTermEffects.unemployment = -0.02;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.gdpGrowth = 0.03; t.longTermEffects.pollution = -0.03;
        t.longTermEffects.happiness = 0.04;
        t.implementationDays = 365*5; t.publicSupport = 0.60; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(80.0); t.yearlyMaintenanceCost = Money(10.0);
        t.groupImpact[SocialGroupType::WORKERS] = 0.5;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.4;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.3;
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = 0.5;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Eletrificação da Frota Nacional";
        t.description = "Proibição de novos veículos a combustão a partir de 2035. Subsídio de R$15mil para compra de EVs, rede de 50mil carregadores públicos e incentivos para 5 fábricas de baterias nacionais. Elimina 30% das emissões de carbono.";
        t.category = LawCategory::TRANSPORTATION;
        t.shortTermEffects.pollution = -0.02; t.shortTermEffects.governmentSpending = 0.03;
        t.shortTermEffects.gdpGrowth = 0.01;
        t.longTermEffects.pollution = -0.08; t.longTermEffects.gdpGrowth = 0.02;
        t.longTermEffects.healthcare = 0.02;
        t.implementationDays = 365*3; t.publicSupport = 0.50; t.parliamentarySupport = 0.45;
        t.implementationCost = Money(40.0); t.yearlyMaintenanceCost = Money(10.0);
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = 0.8;
        t.groupImpact[SocialGroupType::SCIENTISTS] = 0.5;
        t.groupImpact[SocialGroupType::WORKERS] = 0.2;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.2;
        lawTemplates_.push_back(t);
    }

    // ==================== ENERGIA — extras ====================
    {
        Law t; t.name = "Microgeração Distribuída";
        t.description = "Obriga concessionárias a comprar energia solar de pequenos produtores pelo preço de mercado. Tarifas de instalação subsidiadas em 40%. Meta de 30% de microgeração no total da matriz até 2040.";
        t.category = LawCategory::ENERGY;
        t.shortTermEffects.pollution = -0.01; t.shortTermEffects.governmentSpending = 0.01;
        t.shortTermEffects.happiness = 0.01;
        t.longTermEffects.pollution = -0.04; t.longTermEffects.inequality = -0.01;
        t.longTermEffects.gdpGrowth = 0.01;
        t.implementationDays = 180; t.publicSupport = 0.65; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(10.0); t.yearlyMaintenanceCost = Money(3.0);
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.4;
        t.groupImpact[SocialGroupType::UPPER_MIDDLE_CLASS] = 0.6;
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = 0.7;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Exploração de Gás de Xisto";
        t.description = "Autoriza fracking em bacias sedimentares com reservas de 300 TCF. Independência energética total em 10 anos e exportações de US$30B/ano. Risco sísmico (5 tremores >4.0 por ano estimados) e contaminação de lençóis freáticos.";
        t.category = LawCategory::ENERGY;
        t.shortTermEffects.gdpGrowth = 0.02; t.shortTermEffects.pollution = 0.03;
        t.shortTermEffects.taxRevenue = 0.03;
        t.longTermEffects.gdpGrowth = 0.04; t.longTermEffects.pollution = 0.05;
        t.longTermEffects.healthcare = -0.02;
        t.implementationDays = 365*2; t.publicSupport = 0.30; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(30.0); t.yearlyMaintenanceCost = Money(5.0);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.6;
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = -0.9;
        t.groupImpact[SocialGroupType::FARMERS] = -0.5;
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Racionamento Energético de Emergência";
        t.description = "Cortes programados de energia (2-4h/dia) para regiões de maior consumo durante crises. Rodízio obrigatório para indústria pesada e congelamento de tarifas. Usado quando reservatórios ficam abaixo de 20%.";
        t.category = LawCategory::ENERGY;
        t.shortTermEffects.gdpGrowth = -0.02; t.shortTermEffects.happiness = -0.04;
        t.shortTermEffects.stability = -0.02;
        t.longTermEffects.gdpGrowth = -0.01; t.longTermEffects.stability = 0.01;
        t.implementationDays = 7; t.publicSupport = 0.15; t.parliamentarySupport = 0.35;
        t.implementationCost = Money(0.5);
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.6;
        t.groupImpact[SocialGroupType::WORKERS] = -0.4;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = -0.5;
        lawTemplates_.push_back(t);
    }

    // ==================== AGRICULTURA — extras ====================
    {
        Law t; t.name = "Agricultura Orgânica Subsidiada";
        t.description = "Crédito rural 50% mais barato para transição para agricultura orgânica. Certificação gratuita, acesso preferencial a mercados públicos (escolas, hospitais) e prêmio de 30% nos preços. 500mil agricultores familiares beneficiados.";
        t.category = LawCategory::AGRICULTURE;
        t.shortTermEffects.happiness = 0.01; t.shortTermEffects.pollution = -0.01;
        t.shortTermEffects.governmentSpending = 0.02;
        t.longTermEffects.pollution = -0.03; t.longTermEffects.healthcare = 0.01;
        t.longTermEffects.happiness = 0.02;
        t.implementationDays = 365; t.publicSupport = 0.55; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(10.0); t.yearlyMaintenanceCost = Money(5.0);
        t.groupImpact[SocialGroupType::FARMERS] = 0.6;
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = 0.7;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Seguro Rural Universal";
        t.description = "Cobertura de 100% das perdas agrícolas por eventos climáticos extremos (secas, geadas, inundações) custeada pelo governo. Custo anual de R$15B. Estabiliza renda rural e incentiva modernização da produção.";
        t.category = LawCategory::AGRICULTURE;
        t.shortTermEffects.happiness = 0.02; t.shortTermEffects.stability = 0.01;
        t.shortTermEffects.governmentSpending = 0.02;
        t.longTermEffects.happiness = 0.03; t.longTermEffects.gdpGrowth = 0.01;
        t.longTermEffects.stability = 0.02;
        t.implementationDays = 180; t.publicSupport = 0.65; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(15.0);
        t.groupImpact[SocialGroupType::FARMERS] = 0.8;
        t.groupImpact[SocialGroupType::LOWER_CLASS] = 0.3;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Transgênicos e Biotecnologia Agrícola";
        t.description = "Expansão de culturas OGM aprovadas pela CTNBIO, incluindo arroz dourado e milho resistente à seca. Aumenta produtividade 20-40% e resistência climática, mas preocupa consumidores europeus e pode fechar mercados.";
        t.category = LawCategory::AGRICULTURE;
        t.shortTermEffects.gdpGrowth = 0.02; t.shortTermEffects.pollution = -0.01;
        t.shortTermEffects.happiness = -0.01;
        t.longTermEffects.gdpGrowth = 0.03; t.longTermEffects.taxRevenue = 0.02;
        t.implementationDays = 90; t.publicSupport = 0.40; t.parliamentarySupport = 0.60;
        t.implementationCost = Money(2.0); t.yearlyMaintenanceCost = Money(1.0);
        t.groupImpact[SocialGroupType::FARMERS] = 0.5;
        t.groupImpact[SocialGroupType::SCIENTISTS] = 0.6;
        t.groupImpact[SocialGroupType::ENVIRONMENTALISTS] = -0.5;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.4;
        lawTemplates_.push_back(t);
    }

    // ==================== RELAÇÕES EXTERIORES — extras ====================
    {
        Law t; t.name = "Alinhamento com Bloco Ocidental";
        t.description = "Parceria estratégica com EUA/OTAN/UE: bases militares conjuntas, acesso preferencial a tecnologia de ponta, proteção de segurança coletiva e integração financeira. Aumenta proteção mas pode criar conflito com vizinhos e China.";
        t.category = LawCategory::FOREIGN_AFFAIRS;
        t.shortTermEffects.stability = 0.02; t.shortTermEffects.gdpGrowth = 0.01;
        t.shortTermEffects.militaryPower = 0.02;
        t.longTermEffects.stability = 0.04; t.longTermEffects.gdpGrowth = 0.02;
        t.longTermEffects.militaryPower = 0.03;
        t.implementationDays = 365; t.publicSupport = 0.40; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(5.0); t.yearlyMaintenanceCost = Money(3.0);
        t.groupImpact[SocialGroupType::MILITARY] = 0.4;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.5;
        t.groupImpact[SocialGroupType::NATIONALISTS] = -0.5;
        t.groupImpact[SocialGroupType::ACTIVISTS] = -0.3;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Cooperação Sul-Sul e BRICS+";
        t.description = "Aprofundamento da cooperação com países em desenvolvimento: sistema de pagamentos alternativo ao SWIFT, banco de desenvolvimento do Sul, e acordos de tecnologia e energia sem condicionalidades políticas.";
        t.category = LawCategory::FOREIGN_AFFAIRS;
        t.shortTermEffects.gdpGrowth = 0.01; t.shortTermEffects.stability = 0.01;
        t.longTermEffects.gdpGrowth = 0.02; t.longTermEffects.stability = 0.02;
        t.longTermEffects.taxRevenue = 0.01;
        t.implementationDays = 365; t.publicSupport = 0.45; t.parliamentarySupport = 0.50;
        t.implementationCost = Money(3.0); t.yearlyMaintenanceCost = Money(2.0);
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.4;
        t.groupImpact[SocialGroupType::WORKERS] = 0.3;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = 0.3;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.2;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Saída de Organizações Internacionais";
        t.description = "Retirada de tratados multilaterais: OMC, FMI, Acordo de Paris. Liberdade total de política econômica e ambiental, mas perda de acesso a mercados, financiamento preferencial e credibilidade internacional.";
        t.category = LawCategory::FOREIGN_AFFAIRS;
        t.shortTermEffects.gdpGrowth = -0.03; t.shortTermEffects.stability = -0.02;
        t.shortTermEffects.freedom = 0.01;
        t.longTermEffects.gdpGrowth = -0.02; t.longTermEffects.stability = -0.03;
        t.implementationDays = 90; t.publicSupport = 0.20; t.parliamentarySupport = 0.25;
        t.implementationCost = Money(1.0);
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.7;
        t.groupImpact[SocialGroupType::ENTREPRENEURS] = -0.6;
        t.groupImpact[SocialGroupType::ACTIVISTS] = -0.5;
        t.groupImpact[SocialGroupType::UPPER_CLASS] = -0.4;
        lawTemplates_.push_back(t);
    }
    {
        Law t; t.name = "Candidatura a Membro Permanente da ONU";
        t.description = "Campanha diplomática para reforma do Conselho de Segurança e obtenção de assento permanente. Requer 10 anos de diplomacia ativa, contribuição a operações de paz e construção de coalizões. Eleva status global permanentemente.";
        t.category = LawCategory::FOREIGN_AFFAIRS;
        t.shortTermEffects.stability = 0.01; t.shortTermEffects.happiness = 0.01;
        t.shortTermEffects.governmentSpending = 0.02;
        t.longTermEffects.stability = 0.04; t.longTermEffects.happiness = 0.03;
        t.longTermEffects.militaryPower = 0.02;
        t.implementationDays = 365*5; t.publicSupport = 0.60; t.parliamentarySupport = 0.70;
        t.implementationCost = Money(20.0); t.yearlyMaintenanceCost = Money(5.0);
        t.groupImpact[SocialGroupType::NATIONALISTS] = 0.6;
        t.groupImpact[SocialGroupType::STUDENTS] = 0.4;
        t.groupImpact[SocialGroupType::MILITARY] = 0.5;
        t.groupImpact[SocialGroupType::ACTIVISTS] = 0.3;
        lawTemplates_.push_back(t);
    }
}

void LawSystem::update(double deltaTime, const SimDate& currentDate) {
    if (currentDate.hour != 0) return;

    for (auto countryId : world_.getAllCountryIds()) {
        auto& country = world_.getCountry(countryId);
        processLegislation(country, deltaTime, currentDate);
    }
}

void LawSystem::shutdown() {
    std::cout << "[Laws] Shutting down." << std::endl;
}

// ===== Processamento de Legislação =====

void LawSystem::processLegislation(Country& country, double dt, const SimDate& date) {
    for (auto lawId : country.activeLaws) {
        auto it = laws_.find(lawId);
        if (it == laws_.end()) continue;
        auto& law = it->second;

        switch (law.status) {
            case Law::Status::PROPOSED:
                law.status = Law::Status::IN_COMMITTEE;
                break;

            case Law::Status::IN_COMMITTEE:
                // Passa ao debate após alguns dias
                law.implementationProgress++;
                if (law.implementationProgress > 7) {
                    law.status = Law::Status::DEBATING;
                    law.implementationProgress = 0;
                }
                break;

            case Law::Status::DEBATING:
                law.implementationProgress++;
                if (law.implementationProgress > 14) {
                    law.status = Law::Status::VOTING;
                    law.implementationProgress = 0;
                }
                break;

            case Law::Status::VOTING: {
                double support = calculateLawSupport(law, country.id);
                if (support > 0.5) {
                    law.status = Law::Status::APPROVED;
                    law.enactedDate = date;
                    law.implementationProgress = 0;
                } else {
                    law.status = Law::Status::VETOED;
                }
                break;
            }

            case Law::Status::APPROVED:
                implementLaw(law, country, dt);
                break;

            case Law::Status::ACTIVE:
                applyLawEffects(law, country, 1.0);
                checkJudicialReview(law, country);
                break;

            default:
                break;
        }
    }
}

void LawSystem::implementLaw(Law& law, Country& country, double dt) {
    if (law.implementationDays <= 0) law.implementationDays = 30;
    law.implementationProgress++;
    double progress = static_cast<double>(law.implementationProgress) / law.implementationDays;

    if (progress < 1.0) {
        applyLawEffects(law, country, progress);
    } else {
        law.status = Law::Status::ACTIVE;
        law.implementationProgress = law.implementationDays;
    }
}

void LawSystem::applyLawEffects(const Law& law, Country& country, double progress) {
    double factor = 1.0 / 365.0; // Efeito diário

    // Misturar efeitos de curto/longo prazo baseado no progresso
    double shortWeight = std::max(0.0, 1.0 - progress);
    double longWeight = progress;

    auto blend = [&](double s, double l) { return (s * shortWeight + l * longWeight) * factor; };

    country.gdpGrowthRate += blend(law.shortTermEffects.gdpGrowth, law.longTermEffects.gdpGrowth);
    country.inflation += blend(law.shortTermEffects.inflation, law.longTermEffects.inflation);
    country.unemploymentRate += blend(law.shortTermEffects.unemployment, law.longTermEffects.unemployment);
    country.giniCoefficient += blend(law.shortTermEffects.inequality, law.longTermEffects.inequality);
    country.corruption += blend(law.shortTermEffects.corruption, law.longTermEffects.corruption);
    country.internalStability += blend(law.shortTermEffects.stability, law.longTermEffects.stability);
}

void LawSystem::checkJudicialReview(Law& law, const Country& country) {
    if (law.judicialApproval < 0.3 && RandomEngine::instance().chance(0.001)) {
        law.status = Law::Status::REPEALED;
    }
}

// ===== Ações do Jogador =====

bool LawSystem::isLawAlreadyActive(CountryID country, const std::string& lawName) const {
    for (const auto& [id, law] : laws_) {
        if (law.country == country && law.name == lawName) {
            // Bloqueio permanente: qualquer lei que já foi promulgada
            // (APPROVED, ACTIVE, REPEALED, EXPIRED) nunca pode ser re-proposta.
            // Apenas VETOED (nunca chegou a virar lei) permite nova tentativa.
            if (law.status != Law::Status::VETOED) {
                return true;
            }
        }
    }
    return false;
}

std::string LawSystem::getLawBlockReason(CountryID country, const std::string& lawName) const {
    for (const auto& [id, law] : laws_) {
        if (law.country != country || law.name != lawName) continue;
        switch (law.status) {
            case Law::Status::PROPOSED:      return "[em tramitação]";
            case Law::Status::IN_COMMITTEE:  return "[em comissão]";
            case Law::Status::DEBATING:      return "[em debate]";
            case Law::Status::VOTING:        return "[em votação]";
            case Law::Status::APPROVED:      return "[aprovada]";
            case Law::Status::ACTIVE:        return "[em vigor ✓]";
            case Law::Status::REPEALED:      return "[revogada — proposta única]";
            case Law::Status::EXPIRED:       return "[expirada — proposta única]";
            case Law::Status::VETOED:        return ""; // pode re-propor
            default:                         return "[bloqueada]";
        }
    }
    return "";
}

LawID LawSystem::proposeLaw(CountryID country, const Law& law) {
    // Bloquear duplicatas: lei ativa ou em tramitação com mesmo nome
    if (isLawAlreadyActive(country, law.name)) {
        return 0; // 0 = inválido, lei já existe
    }

    LawID id = nextLawId_++;
    Law newLaw = law;
    newLaw.id = id;
    newLaw.country = country;
    newLaw.status = Law::Status::PROPOSED;
    newLaw.implementationProgress = 0;

    laws_[id] = newLaw;
    world_.getCountry(country).activeLaws.push_back(id);
    return id;
}

bool LawSystem::pushLawThroughParliament(LawID id) {
    auto it = laws_.find(id);
    if (it == laws_.end()) return false;
    auto& law = it->second;

    if (law.status == Law::Status::PROPOSED ||
        law.status == Law::Status::IN_COMMITTEE ||
        law.status == Law::Status::DEBATING) {
        law.status = Law::Status::VOTING;
        law.parliamentarySupport -= 0.1; // Forçar é impopular
        return true;
    }
    return false;
}

void LawSystem::revokeLaw(CountryID country, LawID id) {
    auto it = laws_.find(id);
    if (it != laws_.end() && it->second.status == Law::Status::ACTIVE) {
        it->second.status = Law::Status::REPEALED;
    }
}

void LawSystem::amendLaw(LawID id, const Law::Effects& newEffects) {
    auto it = laws_.find(id);
    if (it != laws_.end()) {
        it->second.longTermEffects = newEffects;
        it->second.status = Law::Status::PROPOSED; // Re-debater
        it->second.implementationProgress = 0;
    }
}

// ===== Consultas =====

Law LawSystem::getLaw(LawID id) const {
    auto it = laws_.find(id);
    if (it != laws_.end()) return it->second;
    return {};
}

std::vector<Law> LawSystem::getActiveLaws(CountryID country) const {
    std::vector<Law> result;
    for (const auto& [id, law] : laws_) {
        if (law.country == country && law.status == Law::Status::ACTIVE)
            result.push_back(law);
    }
    return result;
}

std::vector<Law> LawSystem::getPendingLaws(CountryID country) const {
    std::vector<Law> result;
    for (const auto& [id, law] : laws_) {
        if (law.country == country &&
            (law.status == Law::Status::PROPOSED ||
             law.status == Law::Status::IN_COMMITTEE ||
             law.status == Law::Status::DEBATING ||
             law.status == Law::Status::VOTING ||
             law.status == Law::Status::APPROVED)) {
            result.push_back(law);
        }
    }
    return result;
}

std::vector<Law> LawSystem::getLawsByCategory(CountryID country, LawCategory category) const {
    std::vector<Law> result;
    for (const auto& [id, law] : laws_) {
        if (law.country == country && law.category == category)
            result.push_back(law);
    }
    return result;
}

double LawSystem::calculateLawSupport(const Law& law, CountryID country) const {
    auto& c = world_.getCountry(country);
    double support = law.parliamentarySupport;
    support += c.governmentApproval * 0.2;
    support += law.publicSupport * 0.3;
    support += RandomEngine::instance().randDouble(-0.1, 0.1);
    return std::clamp(support, 0.0, 1.0);
}

bool LawSystem::canProposeLaw(CountryID country, const Law& law) const {
    // Verificar pré-requisitos
    for (auto reqId : law.prerequisiteLaws) {
        auto it = laws_.find(reqId);
        if (it == laws_.end() || it->second.status != Law::Status::ACTIVE) return false;
    }
    return true;
}

std::vector<Law> LawSystem::getAvailableLawTemplates(LawCategory category) const {
    std::vector<Law> result;
    for (const auto& tmpl : lawTemplates_) {
        if (category == tmpl.category)
            result.push_back(tmpl);
    }
    if (result.empty()) return lawTemplates_;
    return result;
}

std::vector<Law> LawSystem::getAvailableLawTemplates(LawCategory category, CountryID country) const {
    std::vector<Law> all;
    std::vector<Law> available; // Não propostas ainda
    for (const auto& tmpl : lawTemplates_) {
        if (tmpl.category != category) continue;
        all.push_back(tmpl);
        if (!isLawAlreadyActive(country, tmpl.name))
            available.push_back(tmpl);
    }
    // Se todas já estão ativas/tramitando, retornar todas mesmo assim
    return available.empty() ? all : available;
}

void LawSystem::registerLawTemplate(const Law& templateLaw) {
    lawTemplates_.push_back(templateLaw);
}

} // namespace GPS
