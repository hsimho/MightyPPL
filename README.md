# MightyPPL

This is a C++-based tool for translating MITL + Past + Pnueli formulae into timed automata,
partially based on the original OCaml version of MightyL
(can be found [here](https://verif.ulb.ac.be/mightyl/)) described in
the CAV 2017 paper "[MightyL: A Compositional Translation from MITL to Timed Automata](https://hal.science/hal-01525524)".  

MightyPPL currently supports only *unilateral* constraints (the PSPACE fragment of MITL), so things such
as ```G (p -> F [0, 20) q && F (30, infty) r)``` are allowed but not ```F [10, 20] r```.
Another notable difference is that it adopts the *strict-future* semantics for the until and release operators, so we don't need the "next" operator ```X```.

## Technical details

MightyPPL uses the TA and DBM representations provided by 
[MoniTAal](https://github.com/DEIS-Tools/MoniTAal) and [PARDIBAAL](https://github.com/DEIS-Tools/PARDIBAAL).
It uses a *semi-symbolic* representation where
each transition in a TA is labelled with a BDD representing a Boolean formula over propositions
(instead of a single letter).
The output can be provided

- as individual component TAs; or 
- as the product of component TAs; for performance considerations MightyPPL only constructs
(untimely) reachable locations and transitions.

In the former case, the constructed TAs can then be used with [TChecker](https://github.com/ticktac-project/tchecker).
In the latter case, the product TA can be analysed with [TChecker](https://github.com/ticktac-project/tchecker), or the built-in standard backward fixpoint algoritihm
for Buechi emptiness.

Improvements planned:
- ~~Support of past modalities?~~ done
- CLI for options (choose the backend, etc.)
- CLI for model file?

## Build MightyPPL
[Boost](https://www.boost.org/) ```>= 1.40```, which is needed by [MoniTAal](https://github.com/DEIS-Tools/MoniTAal).
```console
$ git clone git@github.com:hsimho/MightyPPL.git
$ cd MightyPPL
```
Edit ```CMakeLists.txt``` and modify ```set(ANTLR_EXECUTABLE ...)``` to point to [ANTLR](https://www.antlr.org/download.html)'s .jar file (complete Java binaries jar). Then
```console
$ git submodule init
$ git submodule update
```
to check out [BuDDy](https://github.com/SSoelvsten/buddy). If these do not work, try
```console
git submodule add https://github.com/SSoelvsten/buddy external/buddy
```
Finally,
```console
$ mkdir build ; cd build
$ cmake ..
$ make
```

## Cite MightyPPL 

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

## License

LGPL-3.0. The MIT License applies to CMake scripts from "[Getting started with ANTLR in C++](https://github.com/gabriele-tomassetti/antlr-cpp)".

