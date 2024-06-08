# MightyL: A Compositional Translation from MITL to Timed Automata

This is a C++ re-implementation of the MightyL tool, originally written in OCaml (the old version
can be found [here](https://verif.ulb.ac.be/mightyl/)) and described in
our CAV 2017 paper [*MightyL: A Compositional Translation from MITL to Timed Automata*](https://hal.science/hal-01525524).
This new version currently supports only *unilateral* constraints (the PSPACE fragment of MITL), so things such
as ```G (p -> F [0, 20) q && F (30, infty) r)``` are allowed but not ```F [10, 20] r```.
Another notable difference is that it adopts the *strict-future* semantics for the until and release operators, so we don't need the "next" operator ```X```.

## Technical details

This version uses the TA and DBM representations provided by 
[MoniTAal](https://github.com/DEIS-Tools/MoniTAal) and [PARDIBAAL](https://github.com/DEIS-Tools/PARDIBAAL).
It uses a *semi-symbolic* representation where
each transition in a TA is labelled with a BDD representing a Boolean formula over propositions
(instead of a single letter).
The discrete state space (i.e. set of locations) of the product of the component TAs, however, is built explicitly;
for performance considerations we do this only for (untimely) reachable locations and transitions.
The product TA is then used as input to a standard backward fixpoint algoritihm for Buechi
emptiness.

Improvements planned:
- Support of past modalities?
- CLI for options (choose the backend, etc.)
- CLI for model file?

## Build MightyL
[Boost](https://www.boost.org/) ```>= 1.40```, which is needed by [MoniTAal](https://github.com/DEIS-Tools/MoniTAal).
```console
$ git clone git@github.com:hsimho/MightyL.git
$ cd MightyL
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
