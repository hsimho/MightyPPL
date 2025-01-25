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
#include "MitlCheckUniVisitor.h"
#include "MitlToUniVisitor.h"
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

    extern bdd encode(const int i, const int offset, const int bits);

    extern std::vector<std::string> sat_paths;

    extern std::vector<std::string> get_letters(const std::string&);

    extern void allsat_print_handler(char*, int); 

    extern const char* out_file; 
    extern std::optional<bool> out_format;
    extern bool out_flatten;
    extern bool out_fin;

    extern std::pair<std::vector<monitaal::TAwithBDDEdges>, std::string> build_ta_from_atom(const mightypplcpp::MitlParser::AtomContext*);

    extern std::pair<monitaal::TA, std::string> build_ta_from_main(mightypplcpp::MitlParser::MainContext*);

}
