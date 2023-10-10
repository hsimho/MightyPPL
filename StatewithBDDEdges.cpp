#include "StatewithBDDEdges.h"

// #include "types.h"

// #include "TAwithBDDEdges.h"
// #include "errors.h"

#include <pardibaal/Federation.h>
// #include <iostream>
// #include <algorithm>
// #include <cassert>

namespace monitaal {

    bool bdd_symbolic_state_t::do_transition(const bdd_edge_t& bdd_edge) {
        if (bdd_edge.from() != _location) return false;

        // If the transition is not possible, do nothing and return false
        if (not this->satisfies(bdd_edge.guard()))
            return false;

        if (!bdd_edge.guard().empty()) {
            for (auto& c : bdd_edge.guard())
                _federation.restrict(c);
        }

        for (const auto& r : bdd_edge.reset())
            _federation.assign(r, 0);

        _location = bdd_edge.to();
        return true;
    }

    void bdd_symbolic_state_t::do_transition_backward(const bdd_edge_t& bdd_edge) {

        if (bdd_edge.to() == _location) {
            _location = bdd_edge.from();

            this->down();
            this->restrict_to_zero(bdd_edge.reset());
            this->free(bdd_edge.reset());
            this->restrict(bdd_edge.guard());
            this->down();
        }
    }

    bool bdd_concrete_state_t::do_transition(const bdd_edge_t& bdd_edge) {
        if (bdd_edge.from() != _location) {
            _valuation[0] = -1; // if the first is -1 then the valuation is empty/invalid
            return false;
        }
        // If the transition is not possible, do nothing and return false
        for (const auto& c : bdd_edge.guard())
            if (!satisfies(c)) {
                _valuation[0] = -1;
                return false;
            }

        _location = bdd_edge.to();

        for (const auto& r : bdd_edge.reset()) {
            _valuation[r] = 0;
        }

        return true;
    }

    std::map<location_id_t, bdd_symbolic_state_t>::iterator bdd_symbolic_state_map_t::begin() {
        return _states.begin();
    }

    std::map<location_id_t, bdd_symbolic_state_t>::const_iterator bdd_symbolic_state_map_t::begin() const {
        return _states.begin();
    }

    std::map<location_id_t, bdd_symbolic_state_t>::iterator bdd_symbolic_state_map_t::end() {
        return _states.end();
    }

    std::map<location_id_t, bdd_symbolic_state_t>::const_iterator bdd_symbolic_state_map_t::end() const {
        return _states.end();
    }

}
