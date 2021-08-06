******************
Using the Compiler
******************

.. index:: ! commandline compiler, compiler;commandline, ! solc

.. _commandline-compiler:

Using the Commandline Compiler
******************************

.. note::
    This section does not apply to :ref:`solcjs <solcjs>`, not even if it is used in commandline mode.

Basic Usage
-----------

One of the build targets of the Solidity repository is ``solc``, the solidity commandline compiler.
Using ``solc --help`` provides you with an explanation of all options. The compiler can produce various outputs, ranging from simple binaries and assembly over an abstract syntax tree (parse tree) to estimations of gas usage.
If you only want to compile a single file, you run it as ``solc --bin sourceFile.sol`` and it will print the binary. If you want to get some of the more advanced output variants of ``solc``, it is probably better to tell it to output everything to separate files using ``solc -o outputDirectory --bin --ast-compact-json --asm sourceFile.sol``.

Optimizer Options
-----------------

Before you deploy your contract, activate the optimizer when compiling using ``solc --optimize --bin sourceFile.sol``.
By default, the optimizer will optimize the contract assuming it is called 200 times across its lifetime
(more specifically, it assumes each opcode is executed around 200 times).
If you want the initial contract deployment to be cheaper and the later function executions to be more expensive,
set it to ``--optimize-runs=1``. If you expect many transactions and do not care for higher deployment cost and
output size, set ``--optimize-runs`` to a high number.
This parameter has effects on the following (this might change in the future):

- the size of the binary search in the function dispatch routine
- the way constants like large numbers or strings are stored

.. index:: allowed paths, --allow-paths, base path, --base-path

Base Path and Import Remapping
------------------------------

The commandline compiler will automatically read imported files from the filesystem, but
it is also possible to provide :ref:`path redirects <import-remapping>` using ``prefix=path`` in the following way:

.. code-block:: bash

    solc github.com/ethereum/dapp-bin/=/usr/local/lib/dapp-bin/ file.sol

This essentially instructs the compiler to search for anything starting with
``github.com/ethereum/dapp-bin/`` under ``/usr/local/lib/dapp-bin``.
``solc`` will not read files from the filesystem that lie outside of
the remapping targets and outside of the directories where explicitly specified source
files reside, so things like ``import "/etc/passwd";`` only work if you add ``/=/`` as a remapping.

When accessing the filesystem to search for imports, :ref:`paths that do not start with ./
or ../ <relative-imports>` are treated as relative to the directory specified using
``--base-path`` option (or the current working directory if base path is not specified).
Furthermore, the part added via ``--base-path`` will not appear in the contract metadata.

For security reasons the compiler has restrictions on what directories it can access.
Directories of source files specified on the command line and target paths of
remappings are automatically allowed to be accessed by the file reader, but everything
else is rejected by default.
Additional paths (and their subdirectories) can be allowed via the
``--allow-paths /sample/path,/another/sample/path`` switch.
Everything inside the path specified via ``--base-path`` is always allowed.

The above is only a simplification of how the compiler handles import paths.
For a detailed explanation with examples and discussion of corner cases please refer to the section on
:ref:`path resolution <path-resolution>`.

.. index:: ! linker, ! --link, ! --libraries
.. _library-linking:

Library Linking
---------------

If your contracts use :ref:`libraries <libraries>`, you will notice that the bytecode contains substrings of the form ``__$53aea86b7d70b31448b230b20ae141a537$__``. These are placeholders for the actual library addresses.
The placeholder is a 34 character prefix of the hex encoding of the keccak256 hash of the fully qualified library name.
The bytecode file will also contain lines of the form ``// <placeholder> -> <fq library name>`` at the end to help
identify which libraries the placeholders represent. Note that the fully qualified library name
is the path of its source file and the library name separated by ``:``.
You can use ``solc`` as a linker meaning that it will insert the library addresses for you at those points:

Either add ``--libraries "file.sol:Math=0x1234567890123456789012345678901234567890 file.sol:Heap=0xabCD567890123456789012345678901234567890"`` to your command to provide an address for each library (use commas or spaces as separators) or store the string in a file (one library per line) and run ``solc`` using ``--libraries fileName``.

.. note::
    Starting Solidity 0.8.1 accepts ``=`` as separator between library and address, and ``:`` as a separator is deprecated. It will be removed in the future. Currently ``--libraries "file.sol:Math:0x1234567890123456789012345678901234567890 file.sol:Heap:0xabCD567890123456789012345678901234567890"`` will work too.

.. index:: --standard-json, --base-path

If ``solc`` is called with the option ``--standard-json``, it will expect a JSON input (as explained below) on the standard input, and return a JSON output on the standard output. This is the recommended interface for more complex and especially automated uses. The process will always terminate in a "success" state and report any errors via the JSON output.
The option ``--base-path`` is also processed in standard-json mode.

If ``solc`` is called with the option ``--link``, all input files are interpreted to be unlinked binaries (hex-encoded) in the ``__$53aea86b7d70b31448b230b20ae141a537$__``-format given above and are linked in-place (if the input is read from stdin, it is written to stdout). All options except ``--libraries`` are ignored (including ``-o``) in this case.

.. warning::
    Manually linking libraries on the generated bytecode is discouraged because it does not update
    contract metadata. Since metadata contains a list of libraries specified at the time of
    compilation and bytecode contains a metadata hash, you will get different binaries, depending
    on when linking is performed.

    You should ask the compiler to link the libraries at the time a contract is compiled by either
    using the ``--libraries`` option of ``solc`` or the ``libraries`` key if you use the
    standard-JSON interface to the compiler.

.. note::
    The library placeholder used to be the fully qualified name of the library itself
    instead of the hash of it. This format is still supported by ``solc --link`` but
    the compiler will no longer output it. This change was made to reduce
    the likelihood of a collision between libraries, since only the first 36 characters
    of the fully qualified library name could be used.

.. _evm-version:
.. index:: ! EVM version, compile target

Setting the EVM Version to Target
*********************************

When you compile your contract code you can specify the Ethereum virtual machine
version to compile for to avoid particular features or behaviours.

.. warning::

   Compiling for the wrong EVM version can result in wrong, strange and failing
   behaviour. Please ensure, especially if running a private chain, that you
   use matching EVM versions.

On the command line, you can select the EVM version as follows:

.. code-block:: shell

  solc --evm-version <VERSION> contract.sol

In the :ref:`standard JSON interface <compiler-api>`, use the ``"evmVersion"``
key in the ``"settings"`` field:

.. code-block:: javascript

    {
      "sources": {/* ... */},
      "settings": {
        "optimizer": {/* ... */},
        "evmVersion": "<VERSION>"
      }
    }

Target Options
--------------

Below is a list of target EVM versions and the compiler-relevant changes introduced
at each version. Backward compatibility is not guaranteed between each version.

- ``homestead``
   - (oldest version)
- ``tangerineWhistle``
   - Gas cost for access to other accounts increased, relevant for gas estimation and the optimizer.
   - All gas sent by default for external calls, previously a certain amount had to be retained.
- ``spuriousDragon``
   - Gas cost for the ``exp`` opcode increased, relevant for gas estimation and the optimizer.
- ``byzantium``
   - Opcodes ``returndatacopy``, ``returndatasize`` and ``staticcall`` are available in assembly.
   - The ``staticcall`` opcode is used when calling non-library view or pure functions, which prevents the functions from modifying state at the EVM level, i.e., even applies when you use invalid type conversions.
   - It is possible to access dynamic data returned from function calls.
   - ``revert`` opcode introduced, which means that ``revert()`` will not waste gas.
- ``constantinople``
   - Opcodes ``create2`, ``extcodehash``, ``shl``, ``shr`` and ``sar`` are available in assembly.
   - Shifting operators use shifting opcodes and thus need less gas.
- ``petersburg``
   - The compiler behaves the same way as with constantinople.
- ``istanbul``
   - Opcodes ``chainid`` and ``selfbalance`` are available in assembly.
- ``berlin`` (**default**)
   - Gas costs for ``SLOAD``, ``*CALL``, ``BALANCE``, ``EXT*`` and ``SELFDESTRUCT`` increased. The
     compiler assumes cold gas costs for such operations. This is relevant for gas estimation and
     the optimizer.


.. index:: ! standard JSON, ! --standard-json
.. _compiler-api:

Compiler Input and Output JSON Description
******************************************

The recommended way to interface with the Solidity compiler especially for
more complex and automated setups is the so-called JSON-input-output interface.
The same interface is provided by all distributions of the compiler.

The fields are generally subject to change,
some are optional (as noted), but we try to only make backwards compatible changes.

The compiler API expects a JSON formatted input and outputs the compilation result in a JSON formatted output.
The standard error output is not used and the process will always terminate in a "success" state, even
if there were errors. Errors are always reported as part of the JSON output.

The following subsections describe the format through an example.
Comments are of course not permitted and used here only for explanatory purposes.

Input Description
-----------------

.. code-block:: javascript

    {
      // Required: Source code language. Currently supported are "Solidity" and "Yul".
      "language": "Solidity",
      // Required
      "sources":
      {
        // The keys here are the "global" names of the source files,
        // imports can use other files via remappings (see below).
        "myFile.sol":
        {
          // Optional: keccak256 hash of the source file
          // It is used to verify the retrieved content if imported via URLs.
          "keccak256": "0x123...",
          // Required (unless "content" is used, see below): URL(s) to the source file.
          // URL(s) should be imported in this order and the result checked against the
          // keccak256 hash (if available). If the hash doesn't match or none of the
          // URL(s) result in success, an error should be raised.
          // Using the commandline interface only filesystem paths are supported.
          // With the JavaScript interface the URL will be passed to the user-supplied
          // read callback, so any URL supported by the callback can be used.
          "urls":
          [
            "bzzr://56ab...",
            "ipfs://Qma...",
            "/tmp/path/to/file.sol"
            // If files are used, their directories should be added to the command line via
            // `--allow-paths <path>`.
          ]
        },
        "destructible":
        {
          // Optional: keccak256 hash of the source file
          "keccak256": "0x234...",
          // Required (unless "urls" is used): literal contents of the source file
          "content": "contract destructible is owned { function shutdown() { if (msg.sender == owner) selfdestruct(owner); } }"
        }
      },
      // Optional
      "settings":
      {
        // Optional: Stop compilation after the given stage. Currently only "parsing" is valid here
        "stopAfter": "parsing",
        // Optional: Sorted list of remappings
        "remappings": [ ":g=/dir" ],
        // Optional: Optimizer settings
        "optimizer": {
          // Disabled by default.
          // NOTE: enabled=false still leaves some optimizations on. See comments below.
          // WARNING: Before version 0.8.6 omitting the 'enabled' key was not equivalent to setting
          // it to false and would actually disable all the optimizations.
          "enabled": true,
          // Optimize for how many times you intend to run the code.
          // Lower values will optimize more for initial deployment cost, higher
          // values will optimize more for high-frequency usage.
          "runs": 200,
          // Switch optimizer components on or off in detail.
          // The "enabled" switch above provides two defaults which can be
          // tweaked here. If "details" is given, "enabled" can be omitted.
          "details": {
            // The peephole optimizer is always on if no details are given,
            // use details to switch it off.
            "peephole": true,
            // The inliner is always on if no details are given,
            // use details to switch it off.
            "inliner": true,
            // The unused jumpdest remover is always on if no details are given,
            // use details to switch it off.
            "jumpdestRemover": true,
            // Sometimes re-orders literals in commutative operations.
            "orderLiterals": false,
            // Removes duplicate code blocks
            "deduplicate": false,
            // Common subexpression elimination, this is the most complicated step but
            // can also provide the largest gain.
            "cse": false,
            // Optimize representation of literal numbers and strings in code.
            "constantOptimizer": false,
            // The new Yul optimizer. Mostly operates on the code of ABI coder v2
            // and inline assembly.
            // It is activated together with the global optimizer setting
            // and can be deactivated here.
            // Before Solidity 0.6.0 it had to be activated through this switch.
            "yul": false,
            // Tuning options for the Yul optimizer.
            "yulDetails": {
              // Improve allocation of stack slots for variables, can free up stack slots early.
              // Activated by default if the Yul optimizer is activated.
              "stackAllocation": true,
              // Select optimization steps to be applied.
              // Optional, the optimizer will use the default sequence if omitted.
              "optimizerSteps": "dhfoDgvulfnTUtnIf..."
            }
          }
        },
        // Version of the EVM to compile for.
        // Affects type checking and code generation. Can be homestead,
        // tangerineWhistle, spuriousDragon, byzantium, constantinople, petersburg, istanbul or berlin
        "evmVersion": "byzantium",
        // Optional: Change compilation pipeline to go through the Yul intermediate representation.
        // This is a highly EXPERIMENTAL feature, not to be used for production. This is false by default.
        "viaIR": true,
        // Optional: Debugging settings
        "debug": {
          // How to treat revert (and require) reason strings. Settings are
          // "default", "strip", "debug" and "verboseDebug".
          // "default" does not inject compiler-generated revert strings and keeps user-supplied ones.
          // "strip" removes all revert strings (if possible, i.e. if literals are used) keeping side-effects
          // "debug" injects strings for compiler-generated internal reverts, implemented for ABI encoders V1 and V2 for now.
          // "verboseDebug" even appends further information to user-supplied revert strings (not yet implemented)
          "revertStrings": "default"
        },
        // Metadata settings (optional)
        "metadata": {
          // Use only literal content and not URLs (false by default)
          "useLiteralContent": true,
          // Use the given hash method for the metadata hash that is appended to the bytecode.
          // The metadata hash can be removed from the bytecode via option "none".
          // The other options are "ipfs" and "bzzr1".
          // If the option is omitted, "ipfs" is used by default.
          "bytecodeHash": "ipfs"
        },
        // Addresses of the libraries. If not all libraries are given here,
        // it can result in unlinked objects whose output data is different.
        "libraries": {
          // The top level key is the the name of the source file where the library is used.
          // If remappings are used, this source file should match the global path
          // after remappings were applied.
          // If this key is an empty string, that refers to a global level.
          "myFile.sol": {
            "MyLib": "0x123123..."
          }
        },
        // The following can be used to select desired outputs based
        // on file and contract names.
        // If this field is omitted, then the compiler loads and does type checking,
        // but will not generate any outputs apart from errors.
        // The first level key is the file name and the second level key is the contract name.
        // An empty contract name is used for outputs that are not tied to a contract
        // but to the whole source file like the AST.
        // A star as contract name refers to all contracts in the file.
        // Similarly, a star as a file name matches all files.
        // To select all outputs the compiler can possibly generate, use
        // "outputSelection: { "*": { "*": [ "*" ], "": [ "*" ] } }"
        // but note that this might slow down the compilation process needlessly.
        //
        // The available output types are as follows:
        //
        // File level (needs empty string as contract name):
        //   ast - AST of all source files
        //
        // Contract level (needs the contract name or "*"):
        //   abi - ABI
        //   devdoc - Developer documentation (natspec)
        //   userdoc - User documentation (natspec)
        //   metadata - Metadata
        //   ir - Yul intermediate representation of the code before optimization
        //   irOptimized - Intermediate representation after optimization
        //   storageLayout - Slots, offsets and types of the contract's state variables.
        //   evm.assembly - New assembly format
        //   evm.legacyAssembly - Old-style assembly format in JSON
        //   evm.bytecode.functionDebugData - Debugging information at function level
        //   evm.bytecode.object - Bytecode object
        //   evm.bytecode.opcodes - Opcodes list
        //   evm.bytecode.sourceMap - Source mapping (useful for debugging)
        //   evm.bytecode.linkReferences - Link references (if unlinked object)
        //   evm.bytecode.generatedSources - Sources generated by the compiler
        //   evm.deployedBytecode* - Deployed bytecode (has all the options that evm.bytecode has)
        //   evm.deployedBytecode.immutableReferences - Map from AST ids to bytecode ranges that reference immutables
        //   evm.methodIdentifiers - The list of function hashes
        //   evm.gasEstimates - Function gas estimates
        //   ewasm.wast - Ewasm in WebAssembly S-expressions format
        //   ewasm.wasm - Ewasm in WebAssembly binary format
        //
        // Note that using a using `evm`, `evm.bytecode`, `ewasm`, etc. will select every
        // target part of that output. Additionally, `*` can be used as a wildcard to request everything.
        //
        "outputSelection": {
          "*": {
            "*": [
              "metadata", "evm.bytecode" // Enable the metadata and bytecode outputs of every single contract.
              , "evm.bytecode.sourceMap" // Enable the source map output of every single contract.
            ],
            "": [
              "ast" // Enable the AST output of every single file.
            ]
          },
          // Enable the abi and opcodes output of MyContract defined in file def.
          "def": {
            "MyContract": [ "abi", "evm.bytecode.opcodes" ]
          }
        },
        // The modelChecker object is experimental and subject to changes.
        "modelChecker":
        {
          // Chose which contracts should be analyzed as the deployed one.
          "contracts":
          {
            "source1.sol": ["contract1"],
            "source2.sol": ["contract2", "contract3"]
          },
          // Choose whether division and modulo operations should be replaced by
          // multiplication with slack variables. Default is `true`.
          // Using `false` here is recommended if you are using the CHC engine
          // and not using Spacer as the Horn solver (using Eldarica, for example).
          // See the Formal Verification section for a more detailed explanation of this option.
          "divModWithSlacks": true,
          // Choose which model checker engine to use: all (default), bmc, chc, none.
          "engine": "chc",
          // Choose whether to output all unproved targets. The default is `false`.
          "showUnproved": true,
          // Choose which targets should be checked: constantCondition,
          // underflow, overflow, divByZero, balance, assert, popEmptyArray, outOfBounds.
          // If the option is not given all targets are checked by default.
          // See the Formal Verification section for the targets description.
          "targets": ["underflow", "overflow", "assert"],
          // Timeout for each SMT query in milliseconds.
          // If this option is not given, the SMTChecker will use a deterministic
          // resource limit by default.
          // A given timeout of 0 means no resource/time restrictions for any query.
          "timeout": 20000
        }
      }
    }


Output Description
------------------

.. code-block:: javascript

    {
      // Optional: not present if no errors/warnings were encountered
      "errors": [
        {
          // Optional: Location within the source file.
          "sourceLocation": {
            "file": "sourceFile.sol",
            "start": 0,
            "end": 100
          },
          // Optional: Further locations (e.g. places of conflicting declarations)
          "secondarySourceLocations": [
            {
              "file": "sourceFile.sol",
              "start": 64,
              "end": 92,
              "message": "Other declaration is here:"
            }
          ],
          // Mandatory: Error type, such as "TypeError", "InternalCompilerError", "Exception", etc.
          // See below for complete list of types.
          "type": "TypeError",
          // Mandatory: Component where the error originated, such as "general", "ewasm", etc.
          "component": "general",
          // Mandatory ("error" or "warning")
          "severity": "error",
          // Optional: unique code for the cause of the error
          "errorCode": "3141",
          // Mandatory
          "message": "Invalid keyword",
          // Optional: the message formatted with source location
          "formattedMessage": "sourceFile.sol:100: Invalid keyword"
        }
      ],
      // This contains the file-level outputs.
      // It can be limited/filtered by the outputSelection settings.
      "sources": {
        "sourceFile.sol": {
          // Identifier of the source (used in source maps)
          "id": 1,
          // The AST object
          "ast": {}
        }
      },
      // This contains the contract-level outputs.
      // It can be limited/filtered by the outputSelection settings.
      "contracts": {
        "sourceFile.sol": {
          // If the language used has no contract names, this field should equal to an empty string.
          "ContractName": {
            // The Ethereum Contract ABI. If empty, it is represented as an empty array.
            // See https://docs.soliditylang.org/en/develop/abi-spec.html
            "abi": [],
            // See the Metadata Output documentation (serialised JSON string)
            "metadata": "{/* ... */}",
            // User documentation (natspec)
            "userdoc": {},
            // Developer documentation (natspec)
            "devdoc": {},
            // Intermediate representation (string)
            "ir": "",
            // See the Storage Layout documentation.
            "storageLayout": {"storage": [/* ... */], "types": {/* ... */} },
            // EVM-related outputs
            "evm": {
              // Assembly (string)
              "assembly": "",
              // Old-style assembly (object)
              "legacyAssembly": {},
              // Bytecode and related details.
              "bytecode": {
                // Debugging data at the level of functions.
                "functionDebugData": {
                  // Now follows a set of functions including compiler-internal and
                  // user-defined function. The set does not have to be complete.
                  "@mint_13": { // Internal name of the function
                    "entryPoint": 128, // Byte offset into the bytecode where the function starts (optional)
                    "id": 13, // AST ID of the function definition or null for compiler-internal functions (optional)
                    "parameterSlots": 2, // Number of EVM stack slots for the function parameters (optional)
                    "returnSlots": 1 // Number of EVM stack slots for the return values (optional)
                  }
                },
                // The bytecode as a hex string.
                "object": "00fe",
                // Opcodes list (string)
                "opcodes": "",
                // The source mapping as a string. See the source mapping definition.
                "sourceMap": "",
                // Array of sources generated by the compiler. Currently only
                // contains a single Yul file.
                "generatedSources": [{
                  // Yul AST
                  "ast": {/* ... */},
                  // Source file in its text form (may contain comments)
                  "contents":"{ function abi_decode(start, end) -> data { data := calldataload(start) } }",
                  // Source file ID, used for source references, same "namespace" as the Solidity source files
                  "id": 2,
                  "language": "Yul",
                  "name": "#utility.yul"
                }],
                // If given, this is an unlinked object.
                "linkReferences": {
                  "libraryFile.sol": {
                    // Byte offsets into the bytecode.
                    // Linking replaces the 20 bytes located there.
                    "Library1": [
                      { "start": 0, "length": 20 },
                      { "start": 200, "length": 20 }
                    ]
                  }
                }
              },
              "deployedBytecode": {
                /* ..., */ // The same layout as above.
                "immutableReferences": {
                  // There are two references to the immutable with AST ID 3, both 32 bytes long. One is
                  // at bytecode offset 42, the other at bytecode offset 80.
                  "3": [{ "start": 42, "length": 32 }, { "start": 80, "length": 32 }]
                }
              },
              // The list of function hashes
              "methodIdentifiers": {
                "delegate(address)": "5c19a95c"
              },
              // Function gas estimates
              "gasEstimates": {
                "creation": {
                  "codeDepositCost": "420000",
                  "executionCost": "infinite",
                  "totalCost": "infinite"
                },
                "external": {
                  "delegate(address)": "25000"
                },
                "internal": {
                  "heavyLifting()": "infinite"
                }
              }
            },
            // Ewasm related outputs
            "ewasm": {
              // S-expressions format
              "wast": "",
              // Binary format (hex string)
              "wasm": ""
            }
          }
        }
      }
    }


Error Types
~~~~~~~~~~~

1. ``JSONError``: JSON input doesn't conform to the required format, e.g. input is not a JSON object, the language is not supported, etc.
2. ``IOError``: IO and import processing errors, such as unresolvable URL or hash mismatch in supplied sources.
3. ``ParserError``: Source code doesn't conform to the language rules.
4. ``DocstringParsingError``: The NatSpec tags in the comment block cannot be parsed.
5. ``SyntaxError``: Syntactical error, such as ``continue`` is used outside of a ``for`` loop.
6. ``DeclarationError``: Invalid, unresolvable or clashing identifier names. e.g. ``Identifier not found``
7. ``TypeError``: Error within the type system, such as invalid type conversions, invalid assignments, etc.
8. ``UnimplementedFeatureError``: Feature is not supported by the compiler, but is expected to be supported in future versions.
9. ``InternalCompilerError``: Internal bug triggered in the compiler - this should be reported as an issue.
10. ``Exception``: Unknown failure during compilation - this should be reported as an issue.
11. ``CompilerError``: Invalid use of the compiler stack - this should be reported as an issue.
12. ``FatalError``: Fatal error not processed correctly - this should be reported as an issue.
13. ``Warning``: A warning, which didn't stop the compilation, but should be addressed if possible.


.. _compiler-tools:

Compiler Tools
**************

solidity-upgrade
----------------

``solidity-upgrade`` can help you to semi-automatically upgrade your contracts
to breaking language changes. While it does not and cannot implement all
required changes for every breaking release, it still supports the ones, that
would need plenty of repetitive manual adjustments otherwise.

.. note::

    ``solidity-upgrade`` carries out a large part of the work, but your
    contracts will most likely need further manual adjustments. We recommend
    using a version control system for your files. This helps reviewing and
    eventually rolling back the changes made.

.. warning::

    ``solidity-upgrade`` is not considered to be complete or free from bugs, so
    please use with care.

How it Works
~~~~~~~~~~~~

You can pass (a) Solidity source file(s) to ``solidity-upgrade [files]``. If
these make use of ``import`` statement which refer to files outside the
current source file's directory, you need to specify directories that
are allowed to read and import files from, by passing
``--allow-paths [directory]``. You can ignore missing files by passing
``--ignore-missing``.

``solidity-upgrade`` is based on ``libsolidity`` and can parse, compile and
analyse your source files, and might find applicable source upgrades in them.

Source upgrades are considered to be small textual changes to your source code.
They are applied to an in-memory representation of the source files
given. The corresponding source file is updated by default, but you can pass
``--dry-run`` to simulate to whole upgrade process without writing to any file.

The upgrade process itself has two phases. In the first phase source files are
parsed, and since it is not possible to upgrade source code on that level,
errors are collected and can be logged by passing ``--verbose``. No source
upgrades available at this point.

In the second phase, all sources are compiled and all activated upgrade analysis
modules are run alongside compilation. By default, all available modules are
activated. Please read the documentation on
:ref:`available modules <upgrade-modules>` for further details.


This can result in compilation errors that may
be fixed by source upgrades. If no errors occur, no source upgrades are being
reported and you're done.
If errors occur and some upgrade module reported a source upgrade, the first
reported one gets applied and compilation is triggered again for all given
source files. The previous step is repeated as long as source upgrades are
reported. If errors still occur, you can log them by passing ``--verbose``.
If no errors occur, your contracts are up to date and can be compiled with
the latest version of the compiler.

.. _upgrade-modules:

Available Upgrade Modules
~~~~~~~~~~~~~~~~~~~~~~~~~

+----------------------------+---------+--------------------------------------------------+
| Module                     | Version | Description                                      |
+============================+=========+==================================================+
| ``constructor``            | 0.5.0   | Constructors must now be defined using the       |
|                            |         | ``constructor`` keyword.                         |
+----------------------------+---------+--------------------------------------------------+
| ``visibility``             | 0.5.0   | Explicit function visibility is now mandatory,   |
|                            |         | defaults to ``public``.                          |
+----------------------------+---------+--------------------------------------------------+
| ``abstract``               | 0.6.0   | The keyword ``abstract`` has to be used if a     |
|                            |         | contract does not implement all its functions.   |
+----------------------------+---------+--------------------------------------------------+
| ``virtual``                | 0.6.0   | Functions without implementation outside an      |
|                            |         | interface have to be marked ``virtual``.         |
+----------------------------+---------+--------------------------------------------------+
| ``override``               | 0.6.0   | When overriding a function or modifier, the new  |
|                            |         | keyword ``override`` must be used.               |
+----------------------------+---------+--------------------------------------------------+
| ``dotsyntax``              | 0.7.0   | The following syntax is deprecated:              |
|                            |         | ``f.gas(...)()``, ``f.value(...)()`` and         |
|                            |         | ``(new C).value(...)()``. Replace these calls by |
|                            |         | ``f{gas: ..., value: ...}()`` and                |
|                            |         | ``(new C){value: ...}()``.                       |
+----------------------------+---------+--------------------------------------------------+
| ``now``                    | 0.7.0   | The ``now`` keyword is deprecated. Use           |
|                            |         | ``block.timestamp`` instead.                     |
+----------------------------+---------+--------------------------------------------------+
| ``constructor-visibility`` | 0.7.0   | Removes visibility of constructors.              |
|                            |         |                                                  |
+----------------------------+---------+--------------------------------------------------+

Please read :doc:`0.5.0 release notes <050-breaking-changes>`,
:doc:`0.6.0 release notes <060-breaking-changes>`,
:doc:`0.7.0 release notes <070-breaking-changes>` and :doc:`0.8.0 release notes <080-breaking-changes>` for further details.

Synopsis
~~~~~~~~

.. code-block:: none

    Usage: solidity-upgrade [options] contract.sol

    Allowed options:
        --help               Show help message and exit.
        --version            Show version and exit.
        --allow-paths path(s)
                             Allow a given path for imports. A list of paths can be
                             supplied by separating them with a comma.
        --ignore-missing     Ignore missing files.
        --modules module(s)  Only activate a specific upgrade module. A list of
                             modules can be supplied by separating them with a comma.
        --dry-run            Apply changes in-memory only and don't write to input
                             file.
        --verbose            Print logs, errors and changes. Shortens output of
                             upgrade patches.
        --unsafe             Accept *unsafe* changes.



Bug Reports / Feature Requests
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you found a bug or if you have a feature request, please
`file an issue <https://github.com/ethereum/solidity/issues/new/choose>`_ on Github.


Example
~~~~~~~

Assume that you have the following contract in ``Source.sol``:

.. code-block:: Solidity

    pragma solidity >=0.6.0 <0.6.4;
    // This will not compile after 0.7.0
    // SPDX-License-Identifier: GPL-3.0
    contract C {
        // FIXME: remove constructor visibility and make the contract abstract
        constructor() internal {}
    }

    contract D {
        uint time;

        function f() public payable {
            // FIXME: change now to block.timestamp
            time = now;
        }
    }

    contract E {
        D d;

        // FIXME: remove constructor visibility
        constructor() public {}

        function g() public {
            // FIXME: change .value(5) =>  {value: 5}
            d.f.value(5)();
        }
    }



Required Changes
^^^^^^^^^^^^^^^^

The above contract will not compile starting from 0.7.0. To bring the contract up to date with the
current Solidity version, the following upgrade modules have to be executed:
``constructor-visibility``, ``now`` and ``dotsyntax``. Please read the documentation on
:ref:`available modules <upgrade-modules>` for further details.


Running the Upgrade
^^^^^^^^^^^^^^^^^^^

It is recommended to explicitly specify the upgrade modules by using ``--modules`` argument.

.. code-block:: bash

    solidity-upgrade --modules constructor-visibility,now,dotsyntax Source.sol

The command above applies all changes as shown below. Please review them carefully (the pragmas will
have to be updated manually.)

.. code-block:: Solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    abstract contract C {
        // FIXME: remove constructor visibility and make the contract abstract
        constructor() {}
    }

    contract D {
        uint time;

        function f() public payable {
            // FIXME: change now to block.timestamp
            time = block.timestamp;
        }
    }

    contract E {
        D d;

        // FIXME: remove constructor visibility
        constructor() {}

        function g() public {
            // FIXME: change .value(5) =>  {value: 5}
            d.f{value: 5}();
        }
    }
