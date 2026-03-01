# 🌐 Statecraft

> **Statecraft** é um simulador geopolítico de estratégia em tempo real jogado inteiramente no terminal. Você assume o controle de um país real, toma decisões econômicas, diplomáticas, militares e legislativas enquanto o mundo ao redor evolui dinamicamente.

---

## 🖼️ Interface

```
┌─ ◆ GPS ─ Brasil ─ 2026/03/01 ──────────────────────────────────────────────┐
│ PIB: $1.92T  ▲2.9%  Infl: 4.6%  Desemp: 8.0%  ✔ 38.0%  Saldo: +$12.3B ▲  │
└─────────────────────────────────────────────────────────────────────────────┘
 ▣ Painel   ₿ Economia   ⚔ Diplomacia   ★ Militar   ≡ Leis   ◎ Intel.   ☰ Ranking
```

A interface é 100% TUI (Text User Interface) construída com a biblioteca [FTXUI](https://github.com/ArthurSonzogni/FTXUI), com layout em múltiplas colunas, barras de progresso coloridas e navegação por abas.

---

## 🎮 Funcionalidades

### 📊 Painel de Controle
- Visão geral do país em tempo real: PIB, inflação, desemprego, aprovação, dívida pública
- Fatores de impacto na aprovação do governo (crescimento, inflação, corrupção, leis aprovadas)
- Índice de desenvolvimento humano, expectativa de vida, alfabetização
- Monitor de energia, CO₂, qualidade do ar e clima global
- Feed de eventos ativos com severidade visual

### 💰 Gestão Econômica
- Definir alíquotas do imposto de renda (baixa, média, alta), IRPJ, IVA e tarifas de importação
- Controlar gastos governamentais como % do PIB (austeridade ↔ expansionista)
- Distribuir o orçamento entre **13 áreas**: Defesa, Educação, Saúde, Previdência, Ciência, Infraestrutura, Transporte, Ambiente, Segurança Pública, Agricultura, Cultura, Administração e Serviço da Dívida
- Ações fiscais diretas: emitir moeda, emitir dívida, pagar dívida
- **Banco Central automático**: ajusta a taxa de juros a cada 3 meses pela Regra de Taylor, com notificação de alta/corte/manutenção

### 🤝 Diplomacia
- Tabela de relações com todos os países do mundo
- Status visual com indicadores: `★ ALIADO`, `▲ AMIGÁVEL`, `⚔ GUERRA!`, `⛔ EMBARGO`, etc.
- Ações: Acordo Comercial, Aliança Militar, Sanções, Declarar Guerra, Propor Paz, Enviar Ajuda
- Gestão de propostas pendentes recebidas de outros países

### ⚔️ Forças Armadas
- Visão de efetivo ativo, reservistas, ogivas nucleares e poder militar
- Indicadores de treinamento, moral e prontidão
- Criar unidades de Exército, Marinha, Força Aérea, Fuzileiros e Forças Especiais
- Mobilizar/desmobilizar tropas
- Monitor de conflitos ativos e progresso de guerra

### ⚖️ Legislação
- Propor leis de **20 categorias**: Tributária, Econômica, Trabalhista, Previdência, Ambiental, Educação, Criminal, Imigração, Segurança Pública, Direitos Civis, Saúde, Comércio, Militar, Mídia, Tecnologia, Habitação, Transporte, Energia, Agricultura, Relações Exteriores
- Preview de efeitos de curto e longo prazo de cada lei (PIB, inflação, desemprego, felicidade, corrupção, etc.)
- Tramitação parlamentar com apoio proporcional
- Indicador de impacto estimado na aprovação do governo

### 🔍 Inteligência
- Agência de inteligência com capacidades: HUMINT, SIGINT, Cyber, Contra-inteligência
- Operações encobertas: Espionagem, Sabotagem, Propaganda, Desinformação, Interferência Eleitoral, Apoio a Golpe, Ataque Cibernético, Guerra Econômica
- Relatórios de inteligência sobre países alvo
- Recrutar agentes e aumentar orçamento

### 🏆 Ranking Mundial
- Classificação de todos os países por poder total, com medalhas para o Top 3
- Estatísticas globais: PIB mundial, população global, emissões de CO₂, temperatura

---

## 🌍 Países Jogáveis (Demo)

A demo inclui **8 países** modelados com dados reais (2024):

| País | PIB (USD) | Pop. | Sistema |
|---|---|---|---|
| 🇧🇷 Brasil | $1.92T | 215M | República Presidencialista |
| 🇺🇸 EUA | $27.4T | 335M | República Federal |
| 🇨🇳 China | $17.7T | 1410M | República Popular |
| 🇩🇪 Alemanha | $4.46T | 84M | República Parlamentar |
| 🇷🇺 Rússia | $1.86T | 144M | República Federal |
| 🇮🇳 Índia | $3.57T | 1420M | República Parlamentar |
| 🇯🇵 Japão | $4.21T | 124M | Monarquia Constitucional |
| 🇫🇷 França | $3.03T | 68M | República Semipresidencialista |

---

## 🛠️ Compilação

### Requisitos
- **CMake** ≥ 3.14
- **g++** / **clang++** com suporte a C++17
- **git** (para baixar a dependência FTXUI automaticamente)
- Conexão com internet na primeira compilação

### Passos

```bash
git clone https://github.com/juleklazura/statecraft.git
cd statecraft
mkdir build && cd build
cmake ..
make -j$(nproc)
```

O executável gerado fica em `build/gps_game`.

### Executar

```bash
./build/gps_game
```

O jogo **abre automaticamente** em uma janela de terminal dedicada com no mínimo **160 × 44** caracteres. Certifique-se de ter o `gnome-terminal` instalado (padrão no Ubuntu/Fedora com GNOME).

---

## ⌨️ Atalhos de Teclado

| Tecla | Ação |
|---|---|
| `W` | Avançar 1 semana |
| `M` | Avançar 1 mês |
| `Y` | Avançar 1 ano |
| `Q` | Sair do jogo |
| `Tab` / `Setas` | Navegar entre abas e menus |
| `Enter` | Confirmar seleção |

---

## 📁 Estrutura do Projeto

```
statecraft/
├── CMakeLists.txt          # Build system (baixa FTXUI automaticamente)
├── include/
│   ├── core/               # SimulationEngine, Types
│   ├── systems/            # Economia, Diplomacia, Militar, Leis, Intel...
│   ├── ui/                 # GameUI
│   └── world/              # WorldState
├── src/
│   ├── main.cpp            # Ponto de entrada + launcher de janela dedicada
│   ├── core/               # Motor de simulação
│   ├── systems/            # Implementação de cada sistema
│   ├── ui/                 # Interface FTXUI (~2000 linhas)
│   └── world/              # Estado do mundo, países, relações
└── data/                   # Configurações, eventos, leis, cenários (JSON)
```

---

## 🧱 Tecnologias

| Tecnologia | Uso |
|---|---|
| **C++17** | Linguagem principal |
| **CMake** | Build system |
| **[FTXUI](https://github.com/ArthurSonzogni/FTXUI)** | Interface TUI (baixado via FetchContent) |
| **POSIX** | Detecção de tamanho de terminal (`ioctl TIOCGWINSZ`) |

---

## 📜 Licença

MIT © 2026 Gabriel Julek

---

> *"A arte de governar não é escolher entre o bem e o mal, mas entre o prejudicial e o menos prejudicial."* — Charles de Gaulle
