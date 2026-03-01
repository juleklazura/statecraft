/**
 * GPS - Geopolitical Simulator
 * WorldState.cpp - Implementação do estado global do mundo
 */

#include "world/WorldState.h"
#include <stdexcept>

namespace GPS {

// ==================== Países ====================
void WorldState::addCountry(Country country) {
    countries_[country.id] = std::move(country);
}

Country& WorldState::getCountry(CountryID id) {
    auto it = countries_.find(id);
    if (it == countries_.end()) {
        throw std::runtime_error("Country not found: " + std::to_string(id));
    }
    return it->second;
}

const Country& WorldState::getCountry(CountryID id) const {
    auto it = countries_.find(id);
    if (it == countries_.end()) {
        throw std::runtime_error("Country not found: " + std::to_string(id));
    }
    return it->second;
}

std::vector<CountryID> WorldState::getAllCountryIds() const {
    std::vector<CountryID> ids;
    ids.reserve(countries_.size());
    for (const auto& [id, _] : countries_) {
        ids.push_back(id);
    }
    return ids;
}

// ==================== Regiões ====================
void WorldState::addRegion(Region region) {
    regions_[region.id] = std::move(region);
}

Region& WorldState::getRegion(RegionID id) {
    auto it = regions_.find(id);
    if (it == regions_.end()) {
        throw std::runtime_error("Region not found: " + std::to_string(id));
    }
    return it->second;
}

const Region& WorldState::getRegion(RegionID id) const {
    auto it = regions_.find(id);
    if (it == regions_.end()) {
        throw std::runtime_error("Region not found: " + std::to_string(id));
    }
    return it->second;
}

std::vector<RegionID> WorldState::getRegionsOfCountry(CountryID id) const {
    std::vector<RegionID> result;
    for (const auto& [rid, region] : regions_) {
        if (region.countryId == id) {
            result.push_back(rid);
        }
    }
    return result;
}

// ==================== Relações Diplomáticas ====================
DiplomaticRelation& WorldState::getRelation(CountryID a, CountryID b) {
    uint32_t key = makeRelationKey(a, b);
    auto it = relations_.find(key);
    if (it == relations_.end()) {
        // Cria relação neutra por padrão
        DiplomaticRelation rel;
        rel.countryA = std::min(a, b);
        rel.countryB = std::max(a, b);
        rel.status = DiplomaticStatus::NEUTRAL;
        rel.relations = 0.5;
        rel.trust = 0.5;
        relations_[key] = rel;
    }
    return relations_[key];
}

const DiplomaticRelation& WorldState::getRelation(CountryID a, CountryID b) const {
    uint32_t key = makeRelationKey(a, b);
    auto it = relations_.find(key);
    if (it == relations_.end()) {
        static DiplomaticRelation neutral;
        neutral.status = DiplomaticStatus::NEUTRAL;
        neutral.relations = 0.5;
        return neutral;
    }
    return it->second;
}

void WorldState::setRelation(CountryID a, CountryID b, const DiplomaticRelation& rel) {
    uint32_t key = makeRelationKey(a, b);
    relations_[key] = rel;
}

// ==================== Tratados ====================
void WorldState::addTreaty(Treaty treaty) {
    treaties_[treaty.id] = std::move(treaty);
}

Treaty& WorldState::getTreaty(TreatyID id) {
    return treaties_.at(id);
}

std::vector<TreatyID> WorldState::getTreatiesOfCountry(CountryID id) const {
    std::vector<TreatyID> result;
    for (const auto& [tid, treaty] : treaties_) {
        for (auto cid : treaty.signatories) {
            if (cid == id) {
                result.push_back(tid);
                break;
            }
        }
    }
    return result;
}

// ==================== Organizações ====================
void WorldState::addOrganization(InternationalOrganization org) {
    organizations_[org.id] = std::move(org);
}

InternationalOrganization& WorldState::getOrganization(OrganizationID id) {
    return organizations_.at(id);
}

// ==================== Setores ====================
void WorldState::addSector(EconomicSector sector) {
    sectors_[sector.id] = std::move(sector);
}

EconomicSector& WorldState::getSector(SectorID id) {
    return sectors_.at(id);
}

// ==================== Partidos ====================
void WorldState::addParty(PoliticalParty party) {
    parties_[party.id] = std::move(party);
}

PoliticalParty& WorldState::getParty(PartyID id) {
    return parties_.at(id);
}

std::vector<PartyID> WorldState::getPartiesOfCountry(CountryID id) const {
    std::vector<PartyID> result;
    for (const auto& [pid, party] : parties_) {
        if (party.countryId == id) {
            result.push_back(pid);
        }
    }
    return result;
}

// ==================== Grupos Sociais ====================
void WorldState::addSocialGroup(SocialGroup group) {
    socialGroups_[group.id] = std::move(group);
}

SocialGroup& WorldState::getSocialGroup(PopGroupID id) {
    return socialGroups_.at(id);
}

std::vector<PopGroupID> WorldState::getSocialGroupsOfCountry(CountryID id) const {
    std::vector<PopGroupID> result;
    for (const auto& [gid, group] : socialGroups_) {
        if (group.countryId == id) {
            result.push_back(gid);
        }
    }
    return result;
}

// ==================== Indicadores Globais ====================
double WorldState::getGlobalGDP() const {
    double total = 0.0;
    for (const auto& [_, country] : countries_) {
        total += country.gdp.billions;
    }
    return total;
}

double WorldState::getGlobalPopulation() const {
    double total = 0.0;
    for (const auto& [_, country] : countries_) {
        total += country.population;
    }
    return total;
}

double WorldState::getGlobalCO2() const {
    double total = 0.0;
    for (const auto& [_, country] : countries_) {
        total += country.co2EmissionsMT;
    }
    return total;
}

} // namespace GPS
