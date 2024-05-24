grammar Mitl;

@header {

#include "bdd.h"
#include "EnumAtoms.h"

}

main locals [std::map<std::string, int> props; bdd overline; bdd star; bdd tilde; bdd hat]
    : formula EOF
;

formula locals [bool negated = false; bdd overline; bdd star; bdd tilde; bdd hat]
    : atom                                                                  #FormulaAtom
    | Not formula                                                           #FormulaNot
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

atom locals [bool negated = false; int id = 0; enum_atoms type; bdd overline; bdd star; bdd tilde; bdd hat]
    : Finally interval? atom                                                #AtomF
    | Once interval? atom                                                   #AtomO
    
    | Globally interval? atom                                               #AtomG
    | Historically interval? atom                                           #AtomH
    
    | atom Until interval? atom                                             #AtomU       
    | atom Since interval? atom                                             #AtomS
    
    | atom Release interval? atom                                           #AtomR   
    | atom Trigger interval? atom                                           #AtomT

    | PnueliFn interval LParen atoms+=atom (Comma atoms+=atom)+ RParen      #AtomFn
    | PnueliOn interval LParen atoms+=atom (Comma atoms+=atom)+ RParen      #AtomOn

    | PnueliFnDual interval LParen atoms+=atom (Comma atoms+=atom)+ RParen  #AtomFnDual
    | PnueliOnDual interval LParen atoms+=atom (Comma atoms+=atom)+ RParen  #AtomOnDual
    
    | 'true'                                                                #AtomTrue
    | 'false'                                                               #AtomFalse
    | Idfr                                                                  #AtomIdfr
    | LParen formula RParen                                                 #AtomParen

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

IntLit : '0' | ([1-9][0-9]*) ;
Infty : 'infty' ;
Idfr : [a-z][A-Za-z0-9_]* ;

WS : [ \r\n\t]+ -> skip ;

