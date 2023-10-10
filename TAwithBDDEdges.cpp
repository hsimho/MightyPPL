#include "TAwithBDDEdges.h"

namespace monitaal {

    bdd_edge_t::bdd_edge_t(location_id_t from, location_id_t to, const constraints_t& guard, const clocks_t& reset, const bdd_label_t& bdd_label) :
            edge_t(from, to, guard, reset, label_t{}), _bdd_label(bdd_label) {}

    bdd_label_t bdd_edge_t::bdd_label() const { return _bdd_label; }

    TAwithBDDEdges::TAwithBDDEdges(std::string name, clock_map_t clocks, const locations_t &locations, const bdd_edges_t &bdd_edges, location_id_t initial) :
            TA(name, clocks, locations, edges_t{}, initial) {

        bdd_edge_map_t backward_bdd_edges, forward_bdd_edges;

        for (const auto &l : locations) {

            bdd_edges_t to, from;
            for (const auto &e : bdd_edges) {
                if (e.to() == l.id())
                    to.push_back(e);
                if (e.from() == l.id())
                    from.push_back(e);
            }
            backward_bdd_edges.insert({l.id(), to});
            forward_bdd_edges.insert({l.id(), from});

        }

        _backward_bdd_edges = std::move(backward_bdd_edges);
        _forward_bdd_edges = std::move(forward_bdd_edges);
    }

    const bdd_edges_t& TAwithBDDEdges::bdd_edges_to(location_id_t id) const { return _backward_bdd_edges.at(id); }

    const bdd_edges_t& TAwithBDDEdges::bdd_edges_from(location_id_t id) const { return _forward_bdd_edges.at(id); }


    void TAwithBDDEdges::intersection(const TAwithBDDEdges &other) {

        // TODO: This is a temporary adjustment in response to the off-by-one error
        // introduced by https://github.com/DEIS-Tools/MoniTAal/commit/2207cb9
        
        auto clock_size = this->number_of_clocks() - 2;

        std::map<std::pair<location_id_t, location_id_t>, std::pair<location_id_t, location_id_t>> new_loc_indir;
        location_id_t tmp_id = 0;
        for (const auto& [id1, loc1] : this->locations()) {
            for (const auto& [id2, loc2] : other.locations()) {
                new_loc_indir.insert({{loc1.id(), loc2.id()}, {tmp_id, tmp_id+1}});
                tmp_id += 2;
            }
        }

        bdd_edges_t new_bdd_edges;
        for (const auto& [l1, vec1] : this->_forward_bdd_edges)
            for (const auto& e1 : vec1)
                for (const auto& [l2, vec2] : other._forward_bdd_edges)
                    for (const auto& e2 : vec2) {

 //                       std::cout << "(" << l1 << ", " << l2 << ")" << std::endl;
 //                       std::cout << "bdd_satcount(e1.bdd_label()) == " << bdd_satcount(e1.bdd_label()) << std::endl;
 //                       std::cout << "bdd_satcount(e2.bdd_label()) == " << bdd_satcount(e2.bdd_label()) << std::endl;
                        if (bdd_satcount(e1.bdd_label() & e2.bdd_label()) > 0) {

//                            std::cout << "(" << l1 << ", " << l2 << ") syncs" << std::endl;
                            constraints_t guard(e1.guard());
                            for (const auto& c : e2.guard()) {
                                guard.push_back(constraint_t((c._i == 0 ? 0 : c._i + clock_size),
                                                              (c._j == 0 ? 0 : c._j + clock_size), c._bound));
                            }

                            clocks_t reset(e1.reset());
                            for (const auto& r : e2.reset())
                                reset.push_back(r == 0 ? 0 : r + clock_size);

                            bdd new_bdd_label = e1.bdd_label() & e2.bdd_label();

                            const auto& [to_l1, to_l2] = new_loc_indir.at({e1.to(), e2.to()});
                            const auto& [from_l1, from_l2] = new_loc_indir.at({e1.from(), e2.from()});

                            bdd_edge_t new_e1(from_l1, (this->locations().at(e1.from()).is_accept() 
                                                     ? to_l2 : to_l1), guard, reset, new_bdd_label);

                            bdd_edge_t new_e2(from_l2, (other.locations().at(e2.from()).is_accept() 
                                                     ? to_l1 : to_l2), guard, reset, new_bdd_label);

                            new_bdd_edges.push_back(new_e1);
                            new_bdd_edges.push_back(new_e2);
                        }
                    }


        ((TA*)this)->intersection((TA)other);

        bdd_edge_map_t backward_bdd_edges, forward_bdd_edges;

        for (const auto& [_, l] : this->locations()) {

            bdd_edges_t to, from;
            for (const auto &e : new_bdd_edges) {
                if (e.to() == l.id())
                    to.push_back(e);
                if (e.from() == l.id())
                    from.push_back(e);
            }
            backward_bdd_edges.insert({l.id(), to});
            forward_bdd_edges.insert({l.id(), from});

        }

        this->_backward_bdd_edges = std::move(backward_bdd_edges);
        this->_forward_bdd_edges = std::move(forward_bdd_edges);

    }

    TAwithBDDEdges TAwithBDDEdges::intersection(const std::vector<TAwithBDDEdges>& components) {
        
        assert(components.size() > 0);

        if (components.size() == 1) {
            return components[0];
        }

        std::cout << "Doing intersection for " << components.size() << " components..." << std::endl;

        clock_map_t new_clocks;
        std::vector<int> clock_offsets(components.size(), 0);

        int index = 0;
        new_clocks.insert({0, "x0"});

        for (int i = 0; i < components.size(); ++i) {

            // TODO: This is a temporary adjustment in response to the off-by-one error
            // introduced by https://github.com/DEIS-Tools/MoniTAal/commit/2207cb9

            for (int j = 0; j < components[i].number_of_clocks() - 1; ++j) {
                if (j == 0) {
                    clock_offsets[i] = index;
                } else {
                    new_clocks.insert({++index, components[i].clock_name(j) + "_" + std::to_string(i)});
                }
            }
        }

        // std::cout << "Total: " << new_clocks.size() << " clocks." << std::endl;
        

        std::vector<size_t> location_indices(components.size(), 0);
        std::vector<location_id_t> location_ids(components.size(), 0);

        std::map<std::vector<location_id_t>, std::vector<location_id_t>> new_loc_indir;

        location_id_t tmp_id = 0;

        bool incremented = false;

        do {

            for (int i = 0; i < components.size(); ++i) {

                assert(components[i].locations().size() > 0);

                auto it = components[i].locations().begin();
                std::advance(it, location_indices[i]);
                location_ids[i] = it->first;

            }
            
            // std::cout << "location_indices:" << std::endl;
            // for (int i = 0; i < components.size(); ++i) {
            //     std::cout << location_indices[i] << " ";
            // }
            // std::cout << std::endl;

            // std::cout << "location_ids:" << std::endl;
            // for (int i = 0; i < components.size(); ++i) {
            //     std::cout << location_ids[i] << " ";
            // }
            // std::cout << std::endl;

            std::vector<location_id_t> tmp_ids;

            for (const auto& c : components) {
                tmp_ids.push_back(tmp_id++);
            }
            new_loc_indir.insert({location_ids, tmp_ids});

            // std::cout << "tmp_ids:" << std::endl;
            // for (int i = 0; i < components.size(); ++i) {
            //     std::cout << tmp_ids[i] << " ";
            // }
            // std::cout << std::endl;

            incremented = false;
            for (int i = components.size() - 1; !incremented && i >= 0; --i) {

                if (location_indices[i] < components[i].locations().size() - 1) {

                    ++location_indices[i];
                    incremented = true;

                } else {

                    location_indices[i] = 0;

                }

            }

        } while (incremented);

        // At this point location_indices should be all 0 again

        locations_t new_locations;

        incremented = false;

        do {

            for (int i = 0; i < components.size(); ++i) {

                assert(components[i].locations().size() > 0);

                auto it = components[i].locations().begin();
                std::advance(it, location_indices[i]);
                location_ids[i] = it->first;

            }

            constraints_t constr;
            for (int i = 0; i < components.size(); ++i) {

                for (const auto& c : components[i].locations().at(location_ids[i]).invariant()) {
                    constr.push_back(constraint_t((c._i == 0 ? 0 : c._i + clock_offsets[i]),
                                                  (c._j == 0 ? 0 : c._j + clock_offsets[i]), c._bound));
                }

            }

            std::string name;
            for (int i = 0; i < components.size(); ++i) {
                name += components[i].locations().at(location_ids[i]).name() + "_";
            }

            for (int i = 0; i < components.size(); ++i) {

                new_locations.push_back(location_t(components[i].locations().at(location_ids[i]).is_accept(), new_loc_indir.at(location_ids).at(i), name + std::to_string(i), constr));

            }

            incremented = false;
            for (int i = components.size() - 1; !incremented && i >= 0; --i) {


                if (location_indices[i] < components[i].locations().size() - 1) {

                    ++location_indices[i];
                    incremented = true;

                } else {

                    location_indices[i] = 0;

                }

            }

        } while (incremented);

        
        // std::cout << "Synchronising transitions..." << std::endl;
        
        // TODO: Maybe untimed reachability analysis first

        // std::set<location_id_t> reachable_locations;

        bdd_edges_t new_bdd_edges;

        incremented = false;

        do {

            for (int i = 0; i < components.size(); ++i) {

                assert(components[i].locations().size() > 0);

                auto it = components[i].locations().begin();
                std::advance(it, location_indices[i]);
                location_ids[i] = it->first;

            }

            // std::cout << "Sync transitions from:" << std::endl;
            // for (int i = 0; i < components.size(); ++i) {
            //     std::cout << location_ids[i] << " ";
            // }
            // std::cout << std::endl;

            std::vector<size_t> bdd_edge_indices(components.size(), 0);

            bool stucked = false;
            for (int i = 0; i < components.size(); ++i) {

                if (components[i].bdd_edges_from(location_ids[i]).size() == 0) {
                    stucked = true;
                }

            }

            if (stucked) {
                std::cout << "Stucked!" << std::endl;
            }

            if (!stucked) {
                
                do {

                    // std::cout << "bdd_edge_indices:" << std::endl;
                    // for (int i = 0; i < components.size(); ++i) {
                    //     std::cout << bdd_edge_indices[i] << " ";
                    // }
                    // std::cout << std::endl;

                    bdd new_bdd_label = bdd_true();
                    bool contradiction = false;
                    int last_index; 
                    for (int i = 0; i < components.size() && !contradiction; ++i) {
                        new_bdd_label = new_bdd_label & components[i].bdd_edges_from(location_ids[i]).at(bdd_edge_indices[i]).bdd_label();
                        if (new_bdd_label == bdd_false()) {
                            contradiction = true;
                            last_index = i;
                        }
                    }

                    if (!contradiction) {

                        constraints_t guard;
                        for (int i = 0; i < components.size(); ++i) {

                            for (const auto& c : components[i].bdd_edges_from(location_ids[i]).at(bdd_edge_indices[i]).guard()) {
                                guard.push_back(constraint_t((c._i == 0 ? 0 : c._i + clock_offsets[i]),
                                                              (c._j == 0 ? 0 : c._j + clock_offsets[i]), c._bound));
                            }

                        }


                        clocks_t reset;
                        for (int i = 0; i < components.size(); ++i) {

                            for (const auto& r : components[i].bdd_edges_from(location_ids[i]).at(bdd_edge_indices[i]).reset()) {

                                assert(r != 0);
                                reset.push_back(r + clock_offsets[i]);

                            }

                        }

                        std::vector<location_id_t> dest_location_ids(components.size(), 0);

                        for (int i = 0; i < components.size(); ++i) {

                            dest_location_ids[i] = components[i].bdd_edges_from(location_ids[i]).at(bdd_edge_indices[i]).to();

                        }

                        for (int id_src = 0; id_src < components.size(); ++id_src) {


                            if (components[id_src].locations().at(location_ids[id_src]).is_accept()) {

                                if (id_src == components.size() - 1) {

                                    new_bdd_edges.push_back(bdd_edge_t(new_loc_indir.at(location_ids).at(id_src), new_loc_indir.at(dest_location_ids).at(0), guard, reset, new_bdd_label));
                                    // reachable_locations.insert(new_loc_indir.at(dest_location_ids).at(0));

                                    // std::cout << "Adding edge from" << std::endl;
                                    // for (int i = 0; i < components.size(); ++i) {
                                    //     std::cout << location_ids[i] << " ";
                                    // }
                                    // std::cout << "(" << new_loc_indir.at(location_ids).at(id_src) << ")" << std::endl;

                                    // std::cout << "\nto" << std::endl;
                                    // for (int i = 0; i < components.size(); ++i) {
                                    //     std::cout << dest_location_ids[i] << " ";
                                    // }
                                    // std::cout << "(" << new_loc_indir.at(dest_location_ids).at(0) << ")" << std::endl;

                                    // std::cout << "\n(" << id_src << " -> " << 0 << ")" << std::endl;

                                } else {

                                    new_bdd_edges.push_back(bdd_edge_t(new_loc_indir.at(location_ids).at(id_src), new_loc_indir.at(dest_location_ids).at(id_src + 1), guard, reset, new_bdd_label));
                                    // reachable_locations.insert(new_loc_indir.at(dest_location_ids).at(id_src + 1));

                                    // std::cout << "Adding edge from" << std::endl;
                                    // for (int i = 0; i < components.size(); ++i) {
                                    //     std::cout << location_ids[i] << " ";
                                    // }
                                    // std::cout << "(" << new_loc_indir.at(location_ids).at(id_src) << ")" << std::endl;

                                    // std::cout << "\nto" << std::endl;
                                    // for (int i = 0; i < components.size(); ++i) {
                                    //     std::cout << dest_location_ids[i] << " ";
                                    // }
                                    // std::cout << "(" << new_loc_indir.at(dest_location_ids).at(id_src + 1) << ")" << std::endl;

                                    // std::cout << "\n(" << id_src << " -> " << id_src + 1 << ")" << std::endl;

                                }

                            } else {

                                new_bdd_edges.push_back(bdd_edge_t(new_loc_indir.at(location_ids).at(id_src), new_loc_indir.at(dest_location_ids).at(id_src), guard, reset, new_bdd_label));
                                // reachable_locations.insert(new_loc_indir.at(dest_location_ids).at(id_src));

                                // std::cout << "Adding edge from" << std::endl;
                                // for (int i = 0; i < components.size(); ++i) {
                                //     std::cout << location_ids[i] << " ";
                                // }
                                // std::cout << "(" << new_loc_indir.at(location_ids).at(id_src) << ")" << std::endl;

                                // std::cout << "\nto" << std::endl;
                                // for (int i = 0; i < components.size(); ++i) {
                                //     std::cout << dest_location_ids[i] << " ";
                                // }
                                // std::cout << "(" << new_loc_indir.at(dest_location_ids).at(id_src) << ")" << std::endl;

                                // std::cout << "\n(" << id_src << " -> " << id_src << ")" << std::endl;



                            }


                        }

                    }

                    if (!contradiction) {

                        incremented = false;
                        for (int i = components.size() - 1; !incremented && i >= 0; --i) {

                            if (bdd_edge_indices[i] < components[i].bdd_edges_from(location_ids[i]).size() - 1) {

                                ++bdd_edge_indices[i];
                                incremented = true;

                            } else {

                                bdd_edge_indices[i] = 0;

                            }

                        }

                    } else {

                        incremented = false;
                        for (int i = last_index; !incremented && i >= 0; --i) {

                            if (bdd_edge_indices[i] < components[i].bdd_edges_from(location_ids[i]).size() - 1) {

                                ++bdd_edge_indices[i];
                                incremented = true;

                            } else {

                                bdd_edge_indices[i] = 0;

                            }

                        }


                    }

                } while (incremented);
                
            }

            incremented = false;
            for (int i = components.size() - 1; !incremented && i >= 0; --i) {

                if (location_indices[i] < components[i].locations().size() - 1) {

                    ++location_indices[i];
                    incremented = true;

                } else {

                    location_indices[i] = 0;

                }

            }

        } while (incremented);


        for (int i = 0; i < components.size(); ++i) {

            location_ids[i] = components[i].initial_location();

        }

        std::cout << "new_clocks.size() == " << new_clocks.size() << std::endl;
        std::cout << "new_locations.size() == " << new_locations.size() << std::endl;
        std::cout << "new_bdd_edges.size() == " << new_bdd_edges.size() << std::endl;

        // std::cout << "reachable_locations.size() == " << reachable_locations.size() << std::endl;
        std::cout << "Removing the unreachable locations and edges..." << std::endl;


        std::set<location_id_t> new_location_ids_reachable;
        locations_t new_locations_reachable;
        bdd_edges_t new_bdd_edges_reachable;
        
        for (const auto &l : new_locations) {
            if (l.id() == new_loc_indir.at(location_ids).at(0)) {
                new_location_ids_reachable.insert(l.id());
            }
        }

        for (const auto &e : new_bdd_edges) {

            new_location_ids_reachable.insert(e.to());

        }

        for (const auto &e : new_bdd_edges) {

            if (new_location_ids_reachable.count(e.from()) + new_location_ids_reachable.count(e.to()) >= 2) {
                new_bdd_edges_reachable.push_back(e);
            }

        }

        for (const auto &l : new_locations) {
            if (new_location_ids_reachable.count(l.id()) >= 1) {
                new_locations_reachable.push_back(l);
            }
        }

        std::cout << "new_locations_reachable.size() == " << new_locations_reachable.size() << std::endl;
        std::cout << "new_bdd_edges_reachable.size() == " << new_bdd_edges_reachable.size() << std::endl;

        return TAwithBDDEdges("product", new_clocks, new_locations_reachable, new_bdd_edges_reachable, new_loc_indir.at(location_ids).at(0));


    }

    TA TAwithBDDEdges::projection(const std::set<int>& props_to_remove) {

        edges_t edges;
        bdd projected_e;
        bdd new_props = bdd_true();
        for (const auto& i : props_to_remove) {
            new_props = new_props & bdd_ithvar(i);
        }
            
        for (const auto& [id, l] : this->locations()) {

            for (const auto& e : this->bdd_edges_from(id)) {

                std::cout << "Before projection: " << e.bdd_label() << std::endl;

                bdd_allsat(e.bdd_label(), *mightylcpp::allsat_print_handler);
                mightylcpp::sat_paths.clear();

                projected_e = bdd_exist(e.bdd_label(), new_props);

                std::cout << "After projection: " << projected_e << std::endl;

                bdd_allsat(projected_e, *mightylcpp::allsat_print_handler);
                mightylcpp::sat_paths.clear();

                edges.push_back(monitaal::edge_t(e.from(), e.to(), e.guard(), e.reset(), "some_bdd"));     // from, to, guard, reset, label

                // std::set<std::string> labels_set;

                // for (const auto& p : mightylcpp::sat_paths) {
                //     for (const auto& s : mightylcpp::get_letters(p)) {
                //         std::string projected_s;
                //         for (const auto& i : props_to_keep) {
                //             projected_s += s[i];
                //         }
                //         labels_set.insert(projected_s);
                //     }
                // }

                // mightylcpp::sat_paths.clear();

                // for (const auto& label : labels_set) {
                //     edges.push_back(monitaal::edge_t(e.from(), e.to(), e.guard(), e.reset(), label));     // from, to, guard, reset, label
                // }

            }

        }

        clock_map_t clocks;

        // TODO: This is a temporary adjustment in response to the off-by-one error
        // introduced by https://github.com/DEIS-Tools/MoniTAal/commit/2207cb9

        for (clock_index_t i = 0; i < this->number_of_clocks() - 1; ++i) {
            clocks.insert({i, this->clock_name(i)});
        }

        locations_t locations;
        for (const auto& [id, l] : this->locations()) {
            locations.push_back(l);
        }

        std::cout << "Projected TA: " << std::endl;
        std::cout << "clocks.size() == " << clocks.size() << std::endl;
        std::cout << "locations.size() == " << locations.size() << std::endl;
        std::cout << "bdd_edges.size() == " << edges.size() << std::endl;

        return TA("projected", clocks, locations, edges, this->initial_location());

    }

    TAwithBDDEdges TAwithBDDEdges::time_divergence_ta(const bdd& any) {

        clock_map_t clocks;
        clocks.insert({0, "0"});
        clocks.insert({1, "div_clock"});

        locations_t locations;
        locations.push_back(location_t(true, 0, "time_div1", constraints_t{}));
        locations.push_back(location_t(false, 1, "time_div2", constraints_t{}));

        bdd_edges_t bdd_edges;

        bdd_edges.push_back(bdd_edge_t(1, 1, constraints_t{constraint_t::upper_strict(1, 1)}, clocks_t{}, any));
        bdd_edges.push_back(bdd_edge_t(0, 1, constraints_t{}, clocks_t{1}, any));
        bdd_edges.push_back(bdd_edge_t(1, 0, constraints_t{constraint_t::lower_non_strict(1, 1)}, clocks_t{}, any));

        return TAwithBDDEdges("time_divergence", clocks, locations, bdd_edges, 0);

    }

    std::ostream& operator<<(std::ostream& out, const TAwithBDDEdges& T) {

        out << (const TA&)T; 

        out << "\n  BDD Edges:\n";
        for (const auto& es : T._backward_bdd_edges) {
            for (const auto& e : es.second) {
                out << " label: " << '\n';
                bdd_printset(e.bdd_label());
            }
        }

        return out;
    }
}
