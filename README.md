# MightyPPL

This is a tool for translating [MITPPL](https://drops.dagstuhl.de/entities/document/10.4230/LIPIcs.CONCUR.2025.24) formulae over timed words
into (good old pure vanilla) timed automata in


- [Uppaal](https://github.com/UPPAALModelChecker) xml [format](https://docs.uppaal.org/toolsandapi/file-formats/). 
- [TChecker](https://github.com/ticktac-project/tchecker) model [format](https://github.com/ticktac-project/tchecker/wiki/TChecker-file-format).

The most common applications are *satisfiability* and *model checking*: these TAs can
be analysed by Uppaal (for the finite-word case; see also [pyuppaal](https://github.com/bencaldwell/pyuppaal), [pyUPPAAL](https://github.com/Jack0Chan/PyUPPAAL)),
(multi-core) [LTSmin](https://ltsmin.utwente.nl) (for the infinite-word case),
or TChecker.

MITPPL is a very expressive timed logic that contains the full fragment of [MITL](https://dl.acm.org/doi/abs/10.1145/227595.227602) (with [both future and past modalities](https://dl.acm.org/doi/abs/10.1109/SFCS.1992.267774))
and [Pnueli modalities](https://www.sciencedirect.com/science/article/pii/S0304397510001441)
of the form $\mathbf{P \mkern -2mu n}_{[0, u \rangle}(\phi_1, \dots, \phi_n)$ (and their duals and past counterparts).
**Please note that MightyPPL adopts the *strict-future* semantics for all temporal modalities---the *weak-future*
"until" $\mathbf{U}^\textit{w}_I$ can be emulated by, for example when $0 \in I$,
$\varphi_1 \mathbf{U}^\textit{w}_I \varphi_2 \iff \varphi_2 \lor \big(\varphi_1 \land (\varphi_1 \mathbf{U}_I \varphi_2)\big)$.**

Below are some example formulae supported by MightyPPL (all unsatisfiable over infinite timed words):

```
(p1 U[1, 2] p2) && G[1, 3] (!p2)
```
This is unsatisfiable, as you cannot have a $p_2$-event in $t + [1, 2]$ and simultaneously have no $p_2$-event in $t + [1, 3]$.
It becomes satisfiable if the intervals are swapped.
```
Fn[0, 5](p1, p2, p3) && G(p1 -> G[0, 3](!p2)) && G(p2 -> G[0, 3](!p3))
```
Clearly unsatisfiable. If the interval is $[0, 7]$ then satisfiable.

```
F( !((p1 -> (p1 && H [0, 20] p1)) || O [0, 30] (p1 -> (p1 && H [0, 20] p1))) )
```
The formula holds if there is a point where $p_1 \land \textbf{O}_{[0, 20]} (\neg p_1)$
and $\textbf{H}_{[0, 30]} \big( p_1 \land \textbf{O}_{[0, 20]} (\neg p_1) \big)$ both hold. This is impossible
as $\textbf{O}_{[0, 20]} (\neg p_1)$ and $\textbf{H}_{[0, 30]} p_1$ are contradictory.
It becomes satisfiable if $[0, 20]$ and $[0, 30]$ are swapped.


See the grammar file `Mitl.g4` for the exact syntax of input formulae, but in short: `F`, `G`, `U`, `R` as in
[MightyL](https://verif.ulb.ac.be/mightyl/),

- `O` (once) is the past version of `F`,
- `H` (historically) is the past version of `G`,
- `S` (since) is the past version of `U`, and
- `T` (trigger) is the past version of `R`.

Pnueli modalities (and their past and dual versions) are `Fn`, `On`, `Gn` and `Hn`.


## Usage

```
mitppl <in_spec_file> --{fin|inf} [out_file --{tck|xml} [--{noflat|compflat}]] [--debug] [--noback]
```

- `in_spec_file` is a formula, written in plain text as the examples above.
- `--fin` or `--inf` for finite / infinite timed words (finite / Büchi acceptance).
- If `out_file` is specified then either `--tck` or `--xml` must follow.
- Operation modes:
    - The "**noflat**" mode: individual tester / component TAs, one or more for each temporal subformula;
    - The "**compflat**" mode (recommended): individual tester TAs, one for each temporal subformula;
    - The default "**flat**" mode: a single monolithic TA for the entire formula (the synchronous product of the tester and component TAs).
        For performance reasons MightyPPL constructs only forward-reachable state spaces and transitions.
- If `out_file` is unspecifed, then a [standard backward fixpoint algorithm](https://dl.acm.org/doi/abs/10.5555/866681) will be used.
- `--debug` to pause at various points in the processing.
- `--noback` to disable the backward reachability analysis for simplifying tester TAs (no effect
  with `--noflat`).

Currently there are two ways to specify the model $M$ in model checking: edit the generated `xml` /
`tck` file, or hard-code in `MightyPPL.cpp`. See the model-checking benchmarks to see how this is
done.

## Technical details

Internally MightyPPL uses TA and DBM representations provided by 
[MoniTAal](https://github.com/DEIS-Tools/MoniTAal) and [PARDIBAAL](https://github.com/DEIS-Tools/PARDIBAAL).
In particular, TAs are represented *semi-symbolically* where
each transition is labelled with a Boolean formula over propositions
(instead of a single letter).
Instead of involving two-way automata, past modalities are supported by 
leveraging the innocuous fact that *timed regular languages are closed under reversal* (!).
Overlapping obligations from MITL modalities with general intervals and Pnueli
modalities are handled by a novel *sequentialisation* technique; see the [tech
report](https://arxiv.org/abs/2510.01490) for details.

## Build MightyPPL
[Boost](https://www.boost.org/) `>= 1.40` is needed by [MoniTAal](https://github.com/DEIS-Tools/MoniTAal).
```console
$ git clone git@github.com:hsimho/MightyPPL.git
$ cd MightyPPL
```
Edit `CMakeLists.txt` and modify `set(ANTLR_EXECUTABLE ...)` to point to [ANTLR](https://www.antlr.org/download.html)'s .jar file (complete Java binaries jar). Then
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
For the issue with CMake minimum version, edit the following files, modify `cmake_minimum_required(VERSION ...)` to at least `3.5`, 
and `make` again.
```
build/antlr4_runtime/src/antlr4_runtime/runtime/Cpp/_deps/googletest-src/CMakeLists.txt
build/antlr4_runtime/src/antlr4_runtime/runtime/Cpp/_deps/googletest-src/googlemock/CMakeLists.txt
build/antlr4_runtime/src/antlr4_runtime/runtime/Cpp/_deps/googletest-src/googletest/CMakeLists.txt
```

## Cite MightyPPL 

```
@article{hkmmp2025b,
  author    = {Ho, Hsi-Ming and Krishna, Shankara Narayanan and Madnani, Khushraj and Majumdar, Rupak and Pandya, Paritosh},
  title     = {MightyPPL: Verification of MITL with Past and Pnueli Modalities},
  journal   = {CoRR},
  volume    = {abs/2510.01490},
  year      = {2025},
  url       = {https://arxiv.org/abs/2510.01490},
  archivePrefix = {arXiv},
  eprint    = {2510.01490},
}
```

## License

LGPL-3.0. The MIT License applies to CMake scripts from "[Getting started with ANTLR in C++](https://github.com/gabriele-tomassetti/antlr-cpp)".

