
#include "HistoryLog.hpp"

bool item_key_t::operator==(const item_key_t &rhs) const {
    return pd_index == rhs.pd_index &&
           quot == rhs.quot &&
           rem == rhs.rem;
}

bool item_key_t::operator!=(const item_key_t &rhs) const {
    return !(rhs == *this);
}

bool item_key_t::operator<(const item_key_t &rhs) const {
    assert(this->pd_index == rhs.pd_index);
    return (this->quot != rhs.quot) ? this->quot < rhs.quot : this->rem < rhs.rem;
}


bool item_key_t::operator>(const item_key_t &rhs) const {
    return rhs < *this;
}

bool item_key_t::operator<=(const item_key_t &rhs) const {
    return !(rhs < *this);
}

bool item_key_t::operator>=(const item_key_t &rhs) const {
    return !(*this < rhs);
}

std::ostream &operator<<(std::ostream &os, const item_key_t &key) {
    os << "pd_index\t\t quot\t\t rem" << std::endl;
    os << "\t" << key.pd_index << "\t\t" << key.quot << "\t\t" << ((uint64_t) key.rem) << std::endl;
    return os;
}


void flip_quot(item_key_t *item) {
    item->quot = 24 - item->quot;
}

const item_key_t rand_item_key_simple(item_key_t lim_key) {
    return rand_item_key_simple(lim_key.pd_index, lim_key.quot, lim_key.rem);
}

const item_key_t rand_item_key_simple(uint64_t max_pd_index, uint64_t max_quot, uint64_t max_rem) {
    uint64_t pd_index = rand() % max_pd_index;
    uint64_t quot = rand() % max_quot;
    uint64_t rem = rand() % max_rem;
    return {pd_index, quot, rem};
}
