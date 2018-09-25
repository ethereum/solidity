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

Please note that this project is released with a `Contributor Code of Conduct <https://raw.githubusercontent.com/ethereum/solidity/develop/CODE_OF_CONDUCT.md>`_. By participating in this project - in the issues, pull requests, or Gitter channels - you agree to abide by its terms.

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
and instead, ``git rebase`` your branch. This will help us review your change
more easily.

Additionally, if you are writing a new feature, please ensure you add appropriate
test cases under ``test/`` (see below).

However, if you are making a larger change, please consult with the `Solidity Development Gitter channel
<https://gitter.im/ethereum/solidity-dev>`_ (different from the one mentioned above, this one is
focused on compiler and language development instead of language use) first.

New features and bugfixes should be added to the ``Changelog.md`` file: please
follow the style of previous entries, when applicable.

Finally, please make sure you respect the `coding style
<https://raw.githubusercontent.com/ethereum/solidity/develop/CODING_STYLE.md>`_
for this project. Also, even though we do CI testing, please test your code and
ensure that it builds locally before submitting a pull request.

Thank you for your help!

Running the compiler tests
==========================

Solidity includes different types of tests. Most of them are bundled in the application
called ``soltest``. Some of them require the ``aleth`` client in testing mode,
some others require ``libz3`` to be installed.

``soltest`` reads test contracts that are annotated with expected results
stored in ``./test/libsolidity/syntaxTests``. In order for soltest to find these
tests the root test directory has to be specified using the ``--testpath`` command
line option, e.g. ``./build/test/soltest -- --testpath ./test``.

To disable the z3 tests, use ``./build/test/soltest -- --no-smt --testpath ./test`` and
to run a subset of the tests that do not require ``aleth``, use
``./build/test/soltest -- --no-ipc --testpath ./test``.

For all other tests, you need to install `aleth <https://github.com/ethereum/cpp-ethereum/releases/download/solidityTester/eth>`_ and run it in testing mode: ``aleth --test -d /tmp/testeth``.

Then you run the actual tests: ``./build/test/soltest -- --ipcpath /tmp/testeth/geth.ipc --testpath ./test``.

To run a subset of tests, filters can be used:
``soltest -t TestSuite/TestName -- --ipcpath /tmp/testeth/geth.ipc --testpath ./test``,
where ``TestName`` can be a wildcard ``*``.

Alternatively, there is a testing script at ``scripts/tests.sh`` which executes all tests and runs
``aleth`` automatically if it is in the path (but does not download it).

The CI even runs some additional tests (including ``solc-js`` and testing third party Solidity frameworks) that require compiling the Emscripten target.

.. note ::

    Some versions of ``aleth`` cannot be used for testing. We suggest using the same version that is used by the Solidity continuous integration tests.
    Currently the CI uses ``d661ac4fec0aeffbedcdc195f67f5ded0c798278`` of ``aleth``.

Writing and running syntax tests
--------------------------------

As mentioned above, syntax tests are stored in individual files. These files must contain annotations, stating the expected result(s) of the respective test.
The test suite will compile and check them against the given expectations.

Example: ``./test/libsolidity/syntaxTests/double_stateVariable_declaration.sol``

::

    contract test {
        uint256 variable;
        uint128 variable;
    }
    // ----
    // DeclarationError: (36-52): Identifier already declared.

A syntax test must contain at least the contract under test itself, followed by the separator ``// ----``. The following comments are used to describe the
expected compiler errors or warnings. This section can be empty in case the contract should compile without any errors or warnings.

In the above example, the state variable ``variable`` was declared twice, which is not allowed. This will result in a ``DeclarationError`` stating that the identifier was already declared.

The tool that is being used for those tests is called ``isoltest`` and can be found under ``./test/tools/``. It is an interactive tool which allows
editing of failing contracts using your preferred text editor. Let's try to break this test by removing the second declaration of ``variable``:

::

    contract test {
        uint256 variable;
    }
    // ----
    // DeclarationError: (36-52): Identifier already declared.

Running ``./test/isoltest`` again will result in a test failure:

::

    syntaxTests/double_stateVariable_declaration.sol: FAIL
        Contract:
            contract test {
                uint256 variable;
            }

        Expected result:
            DeclarationError: (36-52): Identifier already declared.
        Obtained result:
            Success


``isoltest`` prints the expected result next to the obtained result, but also provides a way to change edit / update / skip the current contract or to even quit.
It offers several options for failing tests:

- edit: ``isoltest`` will try to open the editor that was specified before using ``isoltest --editor /path/to/editor``. If no path was set, this will result in a runtime error. In case an editor was specified, this will open it such that the contract can be adjusted.
- update: Updates the contract under test. This will either remove the annotation which contains the exception not met or will add missing expectations. The test will then be run again.
- skip: Skips the execution of this particular test.
- quit: Quits ``isoltest``.

Automatically updating the test above will change it to

::

    contract test {
        uint256 variable;
    }
    // ----

and re-run the test. It will now pass again:

::

    Re-running test case...
    syntaxTests/double_stateVariable_declaration.sol: OK


.. note::

    Please choose a name for the contract file, that is self-explainatory in the sense of what is been tested, e.g. ``double_variable_declaration.sol``.
    Do not put more than one contract into a single file, unless you are testing inheritance or cross-contract calls.
    Each file should test one aspect of your new feature.


Running the Fuzzer via AFL
==========================

Fuzzing is a technique that runs programs on more or less random inputs to find exceptional execution
states (segmentation faults, exceptions, etc). Modern fuzzers are clever and do a directed search
inside the input. We have a specialized binary called ``solfuzzer`` which takes source code as input
and fails whenever it encounters an internal compiler error, segmentation fault or similar, but
does not fail if e.g. the code contains an error. This way, internal problems in the compiler
can be found by fuzzing tools.

We mainly use `AFL <http://lcamtuf.coredump.cx/afl/>`_ for fuzzing. You need to download and
install AFL packages from your repos (afl, afl-clang) or build them manually.
Next, build Solidity (or just the ``solfuzzer`` binary) with AFL as your compiler:

::

    cd build
    # if needed
    make clean
    cmake .. -DCMAKE_C_COMPILER=path/to/afl-gcc -DCMAKE_CXX_COMPILER=path/to/afl-g++
    make solfuzzer

At this stage you should be able to see a message similar to the following:

::

    Scanning dependencies of target solfuzzer
    [ 98%] Building CXX object test/tools/CMakeFiles/solfuzzer.dir/fuzzer.cpp.o
    afl-cc 2.52b by <lcamtuf@google.com>
    afl-as 2.52b by <lcamtuf@google.com>
    [+] Instrumented 1949 locations (64-bit, non-hardened mode, ratio 100%).
    [100%] Linking CXX executable solfuzzer

If the instrumentation messages did not appear, try switching the cmake flags pointing to AFL's clang binaries:

::

    # if previously failed
    make clean
    cmake .. -DCMAKE_C_COMPILER=path/to/afl-clang -DCMAKE_CXX_COMPILER=path/to/afl-clang++
    make solfuzzer

Othwerise, upon execution the fuzzer will halt with an error saying binary is not instrumented:

::

    afl-fuzz 2.52b by <lcamtuf@google.com>
    ... (truncated messages)
    [*] Validating target binary...

    [-] Looks like the target binary is not instrumented! The fuzzer depends on
        compile-time instrumentation to isolate interesting test cases while
        mutating the input data. For more information, and for tips on how to
        instrument binaries, please see /usr/share/doc/afl-doc/docs/README.

        When source code is not available, you may be able to leverage QEMU
        mode support. Consult the README for tips on how to enable this.
        (It is also possible to use afl-fuzz as a traditional, "dumb" fuzzer.
        For that, you can use the -n option - but expect much worse results.)

    [-] PROGRAM ABORT : No instrumentation detected
             Location : check_binary(), afl-fuzz.c:6920


Next, you need some example source files. This will make it much easier for the fuzzer
to find errors. You can either copy some files from the syntax tests or extract test files
from the documentation or the other tests:

::

    mkdir /tmp/test_cases
    cd /tmp/test_cases
    # extract from tests:
    path/to/solidity/scripts/isolate_tests.py path/to/solidity/test/libsolidity/SolidityEndToEndTest.cpp
    # extract from documentation:
    path/to/solidity/scripts/isolate_tests.py path/to/solidity/docs docs

The AFL documentation states that the corpus (the initial input files) should not be
too large. The files themselves should not be larger than 1 kB and there should be
at most one input file per functionality, so better start with a small number of
input files. There is also a tool called ``afl-cmin`` that can trim input files
that result in similar behaviour of the binary.

Now run the fuzzer (the ``-m`` extends the size of memory to 60 MB):

::

    afl-fuzz -m 60 -i /tmp/test_cases -o /tmp/fuzzer_reports -- /path/to/solfuzzer

The fuzzer will create source files that lead to failures in ``/tmp/fuzzer_reports``.
Often it finds many similar source files that produce the same error. You can
use the tool ``scripts/uniqueErrors.sh`` to filter out the unique errors.

Whiskers
========

*Whiskers* is a string templating system similar to `Mustache <https://mustache.github.io>`_. It is used by the
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
