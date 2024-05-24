# MightyPPL
A tool for translating MITL + Past + Pnueli into timed automata

# MightyL: A Compositional Translation from MITL to Timed Automata

This is a C++ re-implementation of the MightyL tool, originally written in OCaml and described in our CAV 2017 paper [*MightyL: A Compositional Translation from MITL to Timed Automata*](https://hal.science/hal-01525524). The current version supports only *unitary* constraints (the PSPACE fragment of MITL), so things such as ```G (p -> F [0, 20) q && F (30, infty) r)``` are allowed but not ```F [10, 20] r```. Notably, it adopts the *strict-future* semantics for the until and release operators, so we don't need the "next" operator ```X```.


## Build MightyL

Clone the repo. Then edit ```CMakeLists.txt``` and modify ```set(ANTLR_EXECUTABLE ...)``` to point to [ANTLR](https://www.antlr.org/download.html)'s .jar file (complete Java binaries jar).
Then use

```
$ git submodule init
$ git submodule update
```
to check out [BuDDy](https://github.com/SSoelvsten/buddy). If these do not work, try
```
git submodule add https://github.com/SSoelvsten/buddy external/buddy
'''
Finally,
```
$ mkdir build ; cd build
$ cmake ..
$ make
```
