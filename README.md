# MightyL: A Compositional Translation from MITL to Timed Automata

This is a C++ re-implementation of the MightyL tool, originally written in OCaml and described in our CAV 2017 paper [*MightyL: A Compositional Translation from MITL to Timed Automata*](https://hal.science/hal-01525524). The current version supports only *unitary* constraints (the PSPACE fragment of MITL), so things such as ```G (p -> F [0, 20) q && F (30, infty) r)``` are allowed but not ```F [10, 20] r```. Notably, it adopts the *strict-future* semantics for the until and release operators, so we don't need the "next" operator ```X```.


## Build MightyL

The current version makes use of [MoniTAal](https://github.com/DEIS-Tools/MoniTAal), which itself depends on [Boost](https://www.boost.org/) ```>= 1.40```.
```console
$ git clone git@github.com:hsimho/MightyL.git
$ cd MightyL
```
Edit ```CMakeLists.txt``` and modify ```set(ANTLR_EXECUTABLE ...)``` to point to [ANTLR](https://www.antlr.org/download.html)'s .jar file (complete Java binaries jar). Then

```
$ git submodule init
$ git submodule update
```
to check out [BuDDy](https://github.com/SSoelvsten/buddy). Finally,
```
$ mkdir build ; cd build
$ cmake ..
$ make
```

## Cite MightyL 

```latex
@inproceedings{CAV-2017-BrihayeGHM,
	author        = {Thomas Brihaye and Gilles Geeraerts and Hsi-Ming Ho and Benjamin Monmege},
	booktitle     = {Proceedings of the 29th International Conference on Computer Aided Verification, Part I},
	doi           = {10.1007/978-3-319-63387-9_21},
	pages         = {421--440},
	publisher     = {Springer},
	title         = {MightyL: A Compositional Translation from MITL to Timed Automata},
	year          = 2017,
}
```
