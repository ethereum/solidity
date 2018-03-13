############
Contributing
############

Help is always appreciated!

To get started, you can try :ref:`building-from-source` in order to familiarize
yourself with the components of Solidity and the build process. Also, it may be
useful to become well-versed at writing smart-contracts in Solidity.

In particular, we need help in the following areas:

* Improving the documentation
* Responding to questions from other users on `StackExchange
  <https://ethereum.stackexchange.com>`_ and the `Solidity Gitter
  <https://gitter.im/ethereum/solidity>`_
* Fixing and responding to `Solidity's GitHub issues
  <https://github.com/ethereum/solidity/issues>`_, especially those tagged as
  `up-for-grabs <https://github.com/ethereum/solidity/issues?q=is%3Aopen+is%3Aissue+label%3Aup-for-grabs>`_ which are
  meant as introductory issues for external contributors.

How to Report Issues
====================

To report an issue, please use the
`GitHub issues tracker <https://github.com/ethereum/solidity/issues>`_. When
reporting issues, please mention the following details:

* Which version of Solidity you are using
* What was the source code (if applicable)
* Which platform are you running on
* How to reproduce the issue
* What was the result of the issue
* What the expected behaviour is

Reducing the source code that caused the issue to a bare minimum is always
very helpful and sometimes even clarifies a misunderstanding.

Workflow for Pull Requests
==========================

In order to contribute, please fork off of the ``develop`` branch and make your
changes there. Your commit messages should detail *why* you made your change
in addition to *what* you did (unless it is a tiny change).

If you need to pull in any changes from ``develop`` after making your fork (for
example, to resolve potential merge conflicts), please avoid using ``git merge``
and instead, ``git rebase`` your branch.

Additionally, if you are writing a new feature, please ensure you write appropriate
Boost test cases and place them under ``test/``.

However, if you are making a larger change, please consult with the `Solidity Development Gitter channel
<https://gitter.im/ethereum/solidity-dev>`_ (different from the one mentioned above, this on is
focused on compiler and language development instead of language use) first.


Finally, please make sure you respect the `coding standards
<https://raw.githubusercontent.com/ethereum/cpp-ethereum/develop/CodingStandards.txt>`_
for this project. Also, even though we do CI testing, please test your code and
ensure that it builds locally before submitting a pull request.

Thank you for your help!

Running the compiler tests
==========================

Solidity includes different types of tests. They are included in the application
called ``soltest``. Some of them require the ``cpp-ethereum`` client in testing mode,
some others require ``libz3`` to be installed.

``soltest`` reads test contracts that are annotated with expected results
stored in ``./test/libsolidity/syntaxTests``. In order for soltest to find these
tests the root test directory has to be specified using the ``--testpath`` command
line option, e.g. ``./build/test/soltest -- --testpath ./test``.

To disable the z3 tests, use ``./build/test/soltest -- --no-smt --testpath ./test`` and
to run a subset of the tests that do not require ``cpp-ethereum``, use
``./build/test/soltest -- --no-ipc --testpath ./test``.

For all other tests, you need to install `cpp-ethereum <https://github.com/ethereum/cpp-ethereum/releases/download/solidityTester/eth>`_ and run it in testing mode: ``eth --test -d /tmp/testeth``.

Then you run the actual tests: ``./build/test/soltest -- --ipcpath /tmp/testeth/geth.ipc --testpath ./test``.

To run a subset of tests, filters can be used:
``soltest -t TestSuite/TestName -- --ipcpath /tmp/testeth/geth.ipc --testpath ./test``,
where ``TestName`` can be a wildcard ``*``.

Alternatively, there is a testing script at ``scripts/test.sh`` which executes all tests and runs
``cpp-ethereum`` automatically if it is in the path (but does not download it).

Travis CI even runs some additional tests (including ``solc-js`` and testing third party Solidity frameworks) that require compiling the Emscripten target.

Whiskers
========

*Whiskers* is a templating system similar to `Mustache <https://mustache.github.io>`_. It is used by the
compiler in various places to aid readability, and thus maintainability and verifiability, of the code.

The syntax comes with a substantial difference to Mustache: the template markers ``{{`` and ``}}`` are
replaced by ``<`` and ``>`` in order to aid parsing and avoid conflicts with :ref:`inline-assembly`
(The symbols ``<`` and ``>`` are invalid in inline assembly, while ``{`` and ``}`` are used to delimit blocks).
Another limitation is that lists are only resolved one depth and they will not recurse. This may change in the future.

A rough specification is the following:

Any occurrence of ``<name>`` is replaced by the string-value of the supplied variable ``name`` without any
escaping and without iterated replacements. An area can be delimited by ``<#name>...</name>``. It is replaced
by as many concatenations of its contents as there were sets of variables supplied to the template system,
each time replacing any ``<inner>`` items by their respective value. Top-level variables can also be used
inside such areas.
