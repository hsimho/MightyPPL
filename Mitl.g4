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
    
    | Globally interval? atom                                               #AtomG
    
    | atom Until interval? atom                                             #AtomU       
    
    | atom Release interval? atom                                           #AtomR   

/*    
    | Next atom
    | Next LBrack bound Comma bound RBrack atom
    | Next LParen bound Comma bound RBrack atom
    | Next LBrack bound Comma bound RParen atom
    | Next LParen bound Comma bound RParen atom
*/
    
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
Globally : 'G' ;
Until : 'U' ;
Release : 'R' ;
Next : 'X' ;

IntLit : '0' | ([1-9][0-9]*) ;
Infty : 'infty' ;
Idfr : [a-z][A-Za-z0-9_]* ;

WS : [ \r\n\t]+ -> skip ;

