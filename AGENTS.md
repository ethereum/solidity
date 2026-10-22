Follow [CODING_STYLE.md](CODING_STYLE.md) and [CONTRIBUTING.md](CONTRIBUTING.md).

## The language and the compiler

- [List of Classic Solidity features](https://notes.argot.org/@solidity-classic-feature-list):
  check against it that the test cases of a change cover every feature it touches.
- [Guide to adding new EVM opcodes](https://notes.argot.org/@solidity-new-opcode-guide):
  the places a new opcode has to be supported in the compiler.
- [Assorted optimizer ideas](https://notes.argot.org/@solidity-assorted-optimizer-ideas):
  optimizer improvements that have already been discussed.
- Past compiler bugs: [docs/bugs.json](docs/bugs.json) lists them, and the
  [security alerts](https://www.soliditylang.org/blog/category/security-alerts/) on the
  Solidity blog explain them in more detail.

## Process

- [SECURITY.md](SECURITY.md): how security bugs are reported. Do not describe a
  vulnerability in a public issue or pull request.
- [ReviewChecklist.md](ReviewChecklist.md): what a pull request is checked for before it
  is merged.

## Parts of the codebase with their own instructions

- [scripts/docker/buildpack-deps/README.md](scripts/docker/buildpack-deps/README.md): the
  Docker images CI builds and tests with, and how they are versioned.
- [test/evmc/README.md](test/evmc/README.md): how to upgrade the copy of EVMC.
- [test/externalTests/README.md](test/externalTests/README.md): the tests that compile
  open-source projects and run their test suites.
- [test/formal/README.md](test/formal/README.md): the formal proofs of the optimization
  rules.
- [solc-js README](https://github.com/argotorg/solc-js/blob/master/README.md): the
  JavaScript bindings of the compiler, which live in a separate repository.
