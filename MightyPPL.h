#pragma once

#include <sstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iomanip>
#include <optional>

#include "bdd.h"

#include "MitlLexer.h"
#include "MitlParser.h"

#include "MitlFormulaVisitor.h"
#include "MitlTypingVisitor.h"
#include "MitlCheckNNFVisitor.h"
#include "MitlToNNFVisitor.h"
#include "MitlAtomNumberingVisitor.h"
#include "MitlCollectTemporalVisitor.h"
#include "MitlGetBDDVisitor.h"

#include "AtomCmp.h"
#include "EnumAtoms.h"

#include "types.h"
#include "TA.h"

#include "TAwithBDDEdges.h"

namespace mightypplcpp {

    extern size_t num_all_props;
    extern size_t components_counter;

    extern bdd encode(const int i, const int offset, const int bits);

    extern std::vector<std::string> sat_paths;

    extern std::vector<std::string> get_letters(const std::string&);

    extern void allsat_print_handler(char*, int); 

    extern void build_model_from_ta_bdd(const monitaal::TAwithBDDEdges, const size_t, std::stringstream&);
    extern void build_untimed_edge(monitaal::bdd_edges_t&, const std::map<std::string, monitaal::location_id_t>&, std::stringstream&, const std::string&, const std::string&, const std::string&, bdd);
    extern void build_edge(monitaal::bdd_edges_t&, const std::map<std::string, monitaal::location_id_t>&, std::stringstream&, const int, const int, const std::string&, const std::string&, const std::string&, const std::string&, const int, bdd);

    extern const char* out_file; 
    extern std::optional<bool> out_format;
    extern bool out_flatten;
    extern bool comp_flatten;
    extern bool out_fin;
    extern bool debug;

    extern monitaal::TAwithBDDEdges varphi;
    extern monitaal::TAwithBDDEdges varphi_;
    extern monitaal::TAwithBDDEdges div;
    extern std::vector<monitaal::TAwithBDDEdges> temporal_components;
    extern monitaal::TAwithBDDEdges model;

    extern std::map<std::string, std::pair<std::set<size_t>, std::set<size_t>>> acc_map;

    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_finally(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_once(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_globally(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_historically(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_until(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_since(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_release(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_trigger(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_pnuelifn(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_pnuelion(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_pnuelign(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_pnuelihn(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_countfn(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_counton(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_countgn(const MitlParser::AtomContext*);
    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_counthn(const MitlParser::AtomContext*);

    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_ta_from_atom(const mightypplcpp::MitlParser::AtomContext*);

    extern std::pair<monitaal::TA, std::string> build_ta_from_main(mightypplcpp::MitlParser::MainContext*);

}
