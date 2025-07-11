# MightyPPL

This is a C++-based tool for translating MITL + Past + Pnueli formulae into timed automata.
MightyPPL supports the full fragment of MITL (future and past modalities) with Pnueli
modalities of the form $\mathbf{Pn}_{[0, u]}(\phi_1, \dots, \phi_n)$ (and the past counterparts).
Please note that MightyPPL adopts the *strict-future* semantics for all the temporal modalities, so we don't need
the "next" operator $\mathbf{X}$ and the "yesterday" operator $\mathbf{Y}$.
Below are some example formulae supported by MightyPPL (all unsatisfiable):

```
(p1 U[1, 2] p2) && G[1, 3] (!p2)
```
```
F( !((p1 -> (p1 && H [0, 20] p1)) || O [0, 30] (p1 -> (p1 && H [0, 20] p1))) )
```
```
Fn[0, 5](p1, p2, p3) && G(p1 -> G[0, 3](!p2)) && G(p2 -> G[0, 3](!p3))
```

See the grammar file `Mitl.g4` for supported modalities, but in short: `F`, `G`, `U`, `R` as usual,
`O` (once) is the past version of `F`, `H` (historically) is the past version of `G`,
`S` (since) is the past version of `U`, and `T` (trigger) is the past version of `R`.
Pnueli modalities (and their past and dual versions) are denoted by `Fn`, `On`, `Gn` and `Hn`.

## Technical details

Internally MightyPPL uses TA and DBM representations provided by 
[MoniTAal](https://github.com/DEIS-Tools/MoniTAal) and [PARDIBAAL](https://github.com/DEIS-Tools/PARDIBAAL).
In particular, TAs are represented *semi-symbolically* where
each transition is labelled with a BDD representing a Boolean formula over propositions
(instead of a single letter).
The output can be

- individual component TAs (one or several for each temporal subformula); or 
- a single monolithic TA, which is the synchronous product of component TAs. For performance reasons MightyPPL constructs only (forward-)reachable state space and transitions.

In the former case, the constructed TAs can then be used with [TChecker](https://github.com/ticktac-project/tchecker).
In the latter case, the product TA can be analysed with [TChecker](https://github.com/ticktac-project/tchecker),
[Uppaal](https://uppaal.org/) (for the finite-word case), [LTSmin](https://ltsmin.utwente.nl) (for the infinite-word case), or the built-in standard backward fixpoint algoritihm for (Buechi) emptiness.

## Build MightyPPL
[Boost](https://www.boost.org/) ```>= 1.40```, which is needed by [MoniTAal](https://github.com/DEIS-Tools/MoniTAal).
```console
$ git clone git@github.com:hsimho/MightyPPL.git
$ cd MightyPPL
```
Edit ```CMakeLists.txt``` and modify ```set(ANTLR_EXECUTABLE ...)``` to point to [ANTLR](https://www.antlr.org/download.html)'s .jar file (complete Java binaries jar). Then
```console
$ git submodule init
$ git submodule update --force --remote
```
to check out [BuDDy](https://github.com/SSoelvsten/buddy). If these do not work, try
```console
git submodule add -f https://github.com/SSoelvsten/buddy external/buddy
mkdir external/monitaal
```
beforehand. Finally,
```console
$ mkdir build ; cd build
$ cmake ..
$ make
```

## Cite MightyPPL 

Tech report will soon be made available.

## License

LGPL-3.0. The MIT License applies to CMake scripts from "[Getting started with ANTLR in C++](https://github.com/gabriele-tomassetti/antlr-cpp)".

