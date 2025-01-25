grammar Mitl;

@header {

#include "bdd.h"
#include "EnumAtoms.h"

}

main locals [std::map<std::string, int> props; std::map<std::string, int> temporals; std::set<int>
repeats; bdd overline; bdd star; bdd tilde; bdd hat]
    : formula EOF
;

formula locals [bool negated = false; bdd overline; bdd star; bdd tilde; bdd hat]
    : atom                                                                  #FormulaAtom
    | Not atom                                                              #FormulaNot
    | formula And formula                                                   #FormulaAnd
    | formula Or formula                                                    #FormulaOr
    | formula Iff formula                                                   #FormulaIff
    | formula Implies formula                                               #FormulaImplies
;


bound
    : IntLit
    | Infty
;

interval
    : LBrack bound Comma bound RBrack
    | LParen bound Comma bound RBrack
    | LBrack bound Comma bound RParen
    | LParen bound Comma bound RParen
;

atom locals [bool uni = true; size_t max_l; size_t bits = 1; bool negated = false; size_t id; enum_atoms type; bdd overline; bdd star; bdd tilde; bdd hat]
    : Finally interval? atom                                                        #AtomF
    | Once interval? atom                                                           #AtomO
    
    | Globally interval? atom                                                       #AtomG
    | Historically interval? atom                                                   #AtomH
    
    | atom Until interval? atom                                                     #AtomU       
    | atom Since interval? atom                                                     #AtomS
    
    | atom Release interval? atom                                                   #AtomR   
    | atom Trigger interval? atom                                                   #AtomT

    | PnueliFn interval LParen atoms+=atom (Comma atoms+=atom)+ RParen              #AtomFn
    | PnueliOn interval LParen atoms+=atom (Comma atoms+=atom)+ RParen              #AtomOn

    | PnueliFnDual interval LParen atoms+=atom (Comma atoms+=atom)+ RParen          #AtomFnDual
    | PnueliOnDual interval LParen atoms+=atom (Comma atoms+=atom)+ RParen          #AtomOnDual

    | CountFn interval LParen atom Comma atom Comma atom Comma atom RParen          #AtomCFn
    | CountOn interval LParen atom Comma atom Comma atom Comma atom RParen          #AtomCOn

    | CountFnDual interval LParen atom Comma atom Comma atom Comma atom RParen      #AtomCFnDual
    | CountOnDual interval LParen atom Comma atom Comma atom Comma atom RParen      #AtomCOnDual
    
    | 'true'                                                                        #AtomTrue
    | 'false'                                                                       #AtomFalse
    | Idfr                                                                          #AtomIdfr
    | LParen formula RParen                                                         #AtomParen

;

Comma : ',' ;
LParen : '(' ;
RParen : ')' ;
LBrack : '[' ;
RBrack : ']' ;

Not : '!' ;
And : '&&' ;
Or : '||' ;
Iff : '<->';
Implies : '->' ;

Finally : 'F' ;
Once : 'O' ;
Globally : 'G' ;
Historically : 'H' ;
Until : 'U' ;
Since : 'S' ;
Release : 'R' ;
Trigger : 'T' ;
PnueliFn : 'Fn' ;
PnueliOn : 'On' ;
PnueliFnDual : 'Gn' ;
PnueliOnDual : 'Hn' ;
CountFn : 'CFn' ;
CountOn : 'COn' ;
CountFnDual: 'CGn' ;
CountOnDual: 'CHn' ;

IntLit : '0' | ([1-9][0-9]*) ;
Infty : 'infty' ;
Idfr : [a-z][A-Za-z0-9_]* ;

WS : [ \r\n\t]+ -> skip ;

