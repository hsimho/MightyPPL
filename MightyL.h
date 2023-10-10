#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include <iomanip>

#include "bdd.h"

#include "MitlLexer.h"
#include "MitlParser.h"

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

namespace mightylcpp {

    extern int num_all_props;

    extern std::vector<std::string> sat_paths;

    extern std::vector<std::string> get_letters(const std::string&);

    extern void allsat_print_handler(char*, int); 

    extern monitaal::TAwithBDDEdges build_ta_from_atom(const mightylcpp::MitlParser::AtomContext*);

    extern monitaal::TA build_ta_from_main(mightylcpp::MitlParser::MainContext*);

}
