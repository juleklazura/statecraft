/**
 * GPS - Geopolitical Simulator
 * DiplomacySystem.cpp - Implementação do sistema diplomático
 */

#include "systems/diplomacy/DiplomacySystem.h"
#include <iostream>
#include <cmath>

namespace GPS {

DiplomacySystem::DiplomacySystem(WorldState& world, const SimulationConfig& config)
    : world_(world), config_(config) {}

void DiplomacySystem::init() {
    std::cout << "[Diplomacy] Initializing diplomatic relations..." << std::endl;
}

void DiplomacySystem::update(double deltaTime, const SimDate& currentDate) {
    updateRelations(deltaTime);
    processProposals(currentDate);
    if (currentDate.hour == 12) { // Uma vez por dia
        updateOrganizations(deltaTime);
        checkForConflictEscalation(currentDate);
    }
}

void DiplomacySystem::shutdown() {
    std::cout << "[Diplomacy] Shutting down." << std::endl;
}

void DiplomacySystem::updateRelations(double dt) {
    auto countries = world_.getAllCountryIds();
    for (size_t i = 0; i < countries.size(); i++) {
        for (size_t j = i + 1; j < countries.size(); j++) {
            auto& rel = world_.getRelation(countries[i], countries[j]);

            // Relações tendem a voltar ao neutro lentamente
            double drift = (0.5 - rel.relations) * 0.001 * dt;
            rel.relations += drift;

            // Comércio melhora relações
            if (rel.tradeVolume > 0) {
                rel.relations += rel.tradeVolume * 0.0001 * dt;
            }

            // Sanções pioram
            if (rel.hasSanctions) {
                rel.relations -= 0.001 * dt;
            }

            rel.relations = std::clamp(rel.relations, 0.0, 1.0);

            // Atualizar status baseado na relação
            if (rel.relations > 0.8 && rel.hasMilitaryAlliance) {
                rel.status = DiplomaticStatus::ALLIED;
            } else if (rel.relations > 0.65) {
                rel.status = DiplomaticStatus::FRIENDLY;
            } else if (rel.relations > 0.35) {
                rel.status = DiplomaticStatus::NEUTRAL;
            } else if (rel.relations > 0.2) {
                rel.status = DiplomaticStatus::COOL;
            } else if (rel.status != DiplomaticStatus::AT_WAR) {
                rel.status = DiplomaticStatus::HOSTILE;
            }

            // Decair modificadores
            for (auto& mod : rel.modifiers) {
                mod.decay();
            }
            rel.modifiers.erase(
                std::remove_if(rel.modifiers.begin(), rel.modifiers.end(),
                              [](const Modifier& m) { return m.expired(); }),
                rel.modifiers.end()
            );
        }
    }
}

void DiplomacySystem::processProposals(const SimDate& date) {
    for (auto& proposal : pendingProposals_) {
        if (!proposal.pending) continue;

        auto& target = world_.getCountry(proposal.target);

        // IA decide se aceita (para países não controlados pelo jogador)
        if (!target.isPlayerControlled) {
            double acceptance = evaluateProposal(proposal);
            if (RandomEngine::instance().chance(acceptance)) {
                proposal.accepted = true;
                // Aplicar efeitos do tratado
                auto& rel = world_.getRelation(proposal.proposer, proposal.target);
                switch (proposal.type) {
                    case DiplomaticProposal::Type::TRADE_AGREEMENT:
                        rel.hasTradeAgreement = true;
                        rel.relations += 0.1;
                        break;
                    case DiplomaticProposal::Type::MILITARY_ALLIANCE:
                        rel.hasMilitaryAlliance = true;
                        rel.relations += 0.2;
                        break;
                    case DiplomaticProposal::Type::NON_AGGRESSION_PACT:
                        rel.relations += 0.1;
                        break;
                    case DiplomaticProposal::Type::SANCTIONS:
                        rel.hasSanctions = true;
                        rel.relations -= 0.3;
                        break;
                    case DiplomaticProposal::Type::LIFT_SANCTIONS:
                        rel.hasSanctions = false;
                        rel.relations += 0.1;
                        break;
                    default:
                        break;
                }
            }
            proposal.pending = false;
        }
    }

    // Mover propostas processadas para histórico
    for (auto it = pendingProposals_.begin(); it != pendingProposals_.end();) {
        if (!it->pending) {
            proposalHistory_.push_back(*it);
            it = pendingProposals_.erase(it);
        } else {
            ++it;
        }
    }
}

void DiplomacySystem::updateOrganizations(double dt) {
    // Organizações influenciam membros
}

void DiplomacySystem::checkForConflictEscalation(const SimDate& date) {
    auto countries = world_.getAllCountryIds();
    for (size_t i = 0; i < countries.size(); i++) {
        for (size_t j = i + 1; j < countries.size(); j++) {
            auto& rel = world_.getRelation(countries[i], countries[j]);
            if (rel.status == DiplomaticStatus::HOSTILE && rel.relations < 0.1) {
                // Risco de guerra
                auto& c1 = world_.getCountry(countries[i]);
                auto& c2 = world_.getCountry(countries[j]);

                // Vizinhos com relações péssimas
                bool areNeighbors = false;
                for (auto n : c1.neighbors) {
                    if (n == countries[j]) { areNeighbors = true; break; }
                }

                if (areNeighbors && RandomEngine::instance().chance(0.0001 * config_.aiAggression)) {
                    // Possível escalada para guerra
                    rel.relations -= 0.05;
                }
            }
        }
    }
}

double DiplomacySystem::evaluateProposal(const DiplomaticProposal& proposal) const {
    auto& rel = world_.getRelation(proposal.proposer, proposal.target);
    double base = rel.relations;

    switch (proposal.type) {
        case DiplomaticProposal::Type::TRADE_AGREEMENT:
            return base * 0.7 + 0.3; // Geralmente favorável
        case DiplomaticProposal::Type::MILITARY_ALLIANCE:
            return base > 0.6 ? base * 0.5 + 0.2 : 0.1;
        case DiplomaticProposal::Type::PEACE_TREATY:
            return 0.5 + (1.0 - base) * 0.3; // Mais provável se relações são ruins mas não péssimas
        case DiplomaticProposal::Type::AID_PACKAGE:
            return 0.8; // Quase sempre aceito
        case DiplomaticProposal::Type::SANCTIONS:
            return 0.1; // Raramente aceito pelo alvo
        default:
            return base;
    }
}

// ===== Ações do Jogador =====

void DiplomacySystem::sendProposal(const DiplomaticProposal& proposal) {
    auto p = proposal;
    p.acceptanceProbability = evaluateProposal(p);
    pendingProposals_.push_back(p);
}

void DiplomacySystem::declareSanctions(CountryID from, CountryID target) {
    auto& rel = world_.getRelation(from, target);
    rel.hasSanctions = true;
    rel.relations -= 0.2;
    rel.tradeVolume *= 0.3;

    // Aliados do alvo ficam descontentes
    for (auto id : world_.getAllCountryIds()) {
        if (id == from || id == target) continue;
        auto& relWithTarget = world_.getRelation(id, target);
        if (relWithTarget.status == DiplomaticStatus::ALLIED) {
            auto& relWithUs = world_.getRelation(from, id);
            relWithUs.relations -= 0.05;
        }
    }
}

void DiplomacySystem::liftSanctions(CountryID from, CountryID target) {
    auto& rel = world_.getRelation(from, target);
    rel.hasSanctions = false;
    rel.relations += 0.05;
}

void DiplomacySystem::declareWar(CountryID aggressor, CountryID target, const std::string& casusBelli) {
    auto& rel = world_.getRelation(aggressor, target);
    rel.status = DiplomaticStatus::AT_WAR;
    rel.relations = 0.0;
    rel.hasSanctions = true;
    rel.hasTradeAgreement = false;
    rel.tradeVolume = 0.0;
    rel.hasEmbassy = false;
}

void DiplomacySystem::proposePeace(CountryID from, CountryID to) {
    DiplomaticProposal proposal;
    proposal.type = DiplomaticProposal::Type::PEACE_TREATY;
    proposal.proposer = from;
    proposal.target = to;
    proposal.description = "Proposta de paz";
    sendProposal(proposal);
}

void DiplomacySystem::breakDiplomaticRelations(CountryID from, CountryID with) {
    auto& rel = world_.getRelation(from, with);
    rel.hasEmbassy = false;
    rel.relations -= 0.3;
    rel.status = DiplomaticStatus::HOSTILE;
}

void DiplomacySystem::restoreDiplomaticRelations(CountryID from, CountryID with) {
    auto& rel = world_.getRelation(from, with);
    rel.hasEmbassy = true;
    rel.relations += 0.1;
    if (rel.status == DiplomaticStatus::HOSTILE) {
        rel.status = DiplomaticStatus::COOL;
    }
}

void DiplomacySystem::respondToProposal(uint32_t proposalId, bool accept) {
    // Para propostas recebidas pelo jogador
}

void DiplomacySystem::joinOrganization(CountryID country, OrganizationID org) {
    auto& organization = world_.getOrganization(org);
    organization.members.push_back(country);
    world_.getCountry(country).memberships.push_back(org);
}

void DiplomacySystem::leaveOrganization(CountryID country, OrganizationID org) {
    auto& organization = world_.getOrganization(org);
    organization.members.erase(
        std::remove(organization.members.begin(), organization.members.end(), country),
        organization.members.end()
    );
}

void DiplomacySystem::sendForeignAid(CountryID from, CountryID to, Money amount) {
    auto& rel = world_.getRelation(from, to);
    auto& sender = world_.getCountry(from);
    // Não pode enviar mais do que há em reservas
    double actual = std::min(amount.billions, sender.foreignReserves.billions);
    if (actual <= 0.0) return;
    rel.relations += actual * 0.05;
    sender.foreignReserves -= Money(actual);
    if (sender.foreignReserves.billions < 0.0) sender.foreignReserves = Money(0.0);
    auto& receiver = world_.getCountry(to);
    receiver.foreignReserves += Money(actual);
}

void DiplomacySystem::proposeUNResolution(CountryID country, const std::string& title, const std::string& desc) {
    // Simplificado
}

void DiplomacySystem::voteUNResolution(CountryID country, uint32_t resolutionId, int vote) {
    // Simplificado
}

// ===== Consultas =====

DiplomaticRelation DiplomacySystem::getRelation(CountryID a, CountryID b) const {
    return world_.getRelation(a, b);
}

double DiplomacySystem::getRelationScore(CountryID a, CountryID b) const {
    return world_.getRelation(a, b).relations;
}

std::vector<DiplomaticProposal> DiplomacySystem::getPendingProposals(CountryID country) const {
    std::vector<DiplomaticProposal> result;
    for (const auto& p : pendingProposals_) {
        if (p.target == country && p.pending) {
            result.push_back(p);
        }
    }
    return result;
}

std::vector<CountryID> DiplomacySystem::getAllies(CountryID country) const {
    std::vector<CountryID> result;
    for (auto id : world_.getAllCountryIds()) {
        if (id == country) continue;
        auto& rel = world_.getRelation(country, id);
        if (rel.status == DiplomaticStatus::ALLIED) {
            result.push_back(id);
        }
    }
    return result;
}

std::vector<CountryID> DiplomacySystem::getEnemies(CountryID country) const {
    std::vector<CountryID> result;
    for (auto id : world_.getAllCountryIds()) {
        if (id == country) continue;
        auto& rel = world_.getRelation(country, id);
        if (rel.status == DiplomaticStatus::HOSTILE || rel.status == DiplomaticStatus::AT_WAR) {
            result.push_back(id);
        }
    }
    return result;
}

bool DiplomacySystem::isAtWar(CountryID a, CountryID b) const {
    return world_.getRelation(a, b).status == DiplomaticStatus::AT_WAR;
}

bool DiplomacySystem::isAllied(CountryID a, CountryID b) const {
    return world_.getRelation(a, b).status == DiplomaticStatus::ALLIED;
}

double DiplomacySystem::getDiplomaticIsolation(CountryID country) const {
    int totalRelations = 0;
    double totalScore = 0;
    for (auto id : world_.getAllCountryIds()) {
        if (id == country) continue;
        totalScore += world_.getRelation(country, id).relations;
        totalRelations++;
    }
    if (totalRelations == 0) return 1.0;
    double avgRelation = totalScore / totalRelations;
    return 1.0 - avgRelation; // 0 = bem conectado, 1 = isolado
}

std::vector<CountryID> DiplomacySystem::getNeighbors(CountryID country) const {
    return world_.getCountry(country).neighbors;
}

double DiplomacySystem::calculateAcceptanceProbability(const DiplomaticProposal& proposal) const {
    return evaluateProposal(proposal);
}

void DiplomacySystem::calculateDiplomaticInfluence(Country& country) {
    double influence = 0.0;
    influence += std::log10(std::max(1.0, country.gdp.billions)) * 0.3;
    influence += country.military.calculatePowerIndex() * 0.2;
    influence += country.softPower * 0.3;
    influence += country.population / 1000.0 * 0.1;
    influence += country.memberships.size() * 0.02;
    country.globalInfluence = std::clamp(influence, 0.0, 1.0);
}

} // namespace GPS
