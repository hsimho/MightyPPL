# MightyPPL

The tool is meant to be used with [TChecker](https://github.com/ticktac-project/tchecker). 
The current version supports only *unilateral* constraints, so things such as ```G (p -> F [0, 20) q && F (30, infty) r)``` are allowed
but not ```F [10, 20] r```. Notably, it adopts the *strict-future* semantics for the until and release operators, so we don't need the "next" operator ```X```.


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
```
Finally,
```
$ mkdir build ; cd build
$ cmake ..
$ make
```
