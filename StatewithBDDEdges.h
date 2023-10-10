#pragma once

#include "TAwithBDDEdges.h"
#include "types.h"
#include "state.h"

#include <map>
#include <vector>


namespace monitaal {

    class bdd_symbolic_state_t : public symbolic_state_t {

    public: 

        using symbolic_state_t::symbolic_state_t;

        bool do_transition(const bdd_edge_t& bdd_edge);
        void do_transition_backward(const bdd_edge_t& bdd_edge);

    private:
        location_id_t _location;
        Federation _federation;

    };

    class bdd_symbolic_state_map_t : public symbolic_state_map_t {

    public:

        [[nodiscard]] std::map<location_id_t, bdd_symbolic_state_t>::iterator begin();
        [[nodiscard]] std::map<location_id_t, bdd_symbolic_state_t>::const_iterator begin() const;

        [[nodiscard]] std::map<location_id_t, bdd_symbolic_state_t>::iterator end();
        [[nodiscard]] std::map<location_id_t, bdd_symbolic_state_t>::const_iterator end() const;

    private:
        std::map<location_id_t, bdd_symbolic_state_t> _states;

    };

    class bdd_concrete_state_t : public concrete_state_t {

    public:

        bool do_transition(const bdd_edge_t& bdd_edge);

    private:
        location_id_t _location;
        valuation_t _valuation;

    };
}
