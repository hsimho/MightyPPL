#include "FixpointwithBDDEdges.h"

namespace monitaal {

    bdd_symbolic_state_map_t FixpointwithBDDEdges::reach(const bdd_symbolic_state_map_t &states, const TAwithBDDEdges& T) {
        bdd_symbolic_state_map_t waiting;
        bdd_symbolic_state_map_t passed;

        // We have to take at least one step
        for (const auto& [_, s] : states) {
            for (const auto& e : T.bdd_edges_to(s.location())) {
                auto state = s;
                state.do_transition_backward(e);
                state.restrict(T.locations().at(e.from()).invariant());
                waiting.insert(state); //Checks for emptyness of the state before inserting
            }
        }

        while (not waiting.is_empty()) {
            bdd_symbolic_state_t s = waiting.begin()->second;
            waiting.remove(s.location());

            if (passed.has_state(s.location()) && s.is_included_in(passed.at(s.location())))
                continue;

            passed.insert(s);

            for (const auto& e : T.bdd_edges_to(s.location())) {
                auto pred = s;
                pred.do_transition_backward(e);
                pred.restrict(T.locations().at(e.from()).invariant());
                waiting.insert(pred);
            }
        }

        return passed;
    }

    bdd_symbolic_state_map_t FixpointwithBDDEdges::accept_states(const TAwithBDDEdges &T) {
        bdd_symbolic_state_map_t accept_states;

        for (const auto& [_, loc] : T.locations()) {
            if (loc.is_accept()) {

                // TODO: This is a temporary adjustment in response to the off-by-one error
                // introduced by https://github.com/DEIS-Tools/MoniTAal/commit/2207cb9

                accept_states.insert(bdd_symbolic_state_t(loc.id(), Federation::unconstrained(T.number_of_clocks() - 1)));
            }
        }

        return accept_states;
    }

    bdd_symbolic_state_map_t FixpointwithBDDEdges::buchi_accept_fixpoint(const TAwithBDDEdges &T) {
        auto reach_a = reach(accept_states(T), T);


        std::vector<location_id_t> erase_list{};
        // Remove states in all locations that are not accept. This is the same as intersecting with accept states
        for (const auto &[l,_] : reach_a)
            if (not T.locations().at(l).is_accept())
                erase_list.push_back(l);

        for (const auto &l : erase_list)
            reach_a.remove(l);
        erase_list.clear();

        auto reach_b = reach(reach_a, T);

        while (not reach_a.equals(reach_b)) {
            reach_a = reach_b;

            for (const auto &[l,_] : reach_b)
                if (not T.locations().at(l).is_accept())
                    erase_list.push_back(l);

            for (const auto &l : erase_list)
                reach_b.remove(l);
            erase_list.clear();

            reach_b = reach(reach_b, T);
        }

        return reach_a;
    }



}
