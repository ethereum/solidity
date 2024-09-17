This is the directory with the Coq files for the translation from Solidity to Coq.

## Generate the test files

## Compile the Coq files

Go to the folder `CoqOfSolidity/` and run:

```sh
make
```

To compile the generated test files you need to use a dedicated command instead of the `Makefile` above, as `coqdep` is taking too long on this folder (several minutes). Go to the current directory and run:

```sh
python scripts/compile_tests.py
```
