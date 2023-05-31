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

One of the build targets of the Solidity repository is ``solc``, the Solidity commandline compiler.
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

.. index:: allowed paths, --allow-paths, base path, --base-path, include paths, --include-path

Base Path and Import Remapping
------------------------------

The commandline compiler will automatically read imported files from the filesystem, but
it is also possible to provide :ref:`path redirects <import-remapping>` using ``prefix=path`` in the following way:

.. code-block:: bash

    solc github.com/ethereum/dapp-bin/=/usr/local/lib/dapp-bin/ file.sol

This essentially instructs the compiler to search for anything starting with
``github.com/ethereum/dapp-bin/`` under ``/usr/local/lib/dapp-bin``.

When accessing the filesystem to search for imports, :ref:`paths that do not start with ./
or ../ <direct-imports>` are treated as relative to the directories specified using
``--base-path`` and ``--include-path`` options (or the current working directory if base path is not specified).
Furthermore, the part of the path added via these options will not appear in the contract metadata.

For security reasons the compiler has :ref:`restrictions on what directories it can access <allowed-paths>`.
Directories of source files specified on the command-line and target paths of
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
version to compile for to avoid particular features or behaviors.

.. warning::

   Compiling for the wrong EVM version can result in wrong, strange and failing
   behavior. Please ensure, especially if running a private chain, that you
   use matching EVM versions.

On the command-line, you can select the EVM version as follows:

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

- ``homestead`` (*support deprecated*)
   - (oldest version)
- ``tangerineWhistle`` (*support deprecated*)
   - Gas cost for access to other accounts increased, relevant for gas estimation and the optimizer.
   - All gas sent by default for external calls, previously a certain amount had to be retained.
- ``spuriousDragon`` (*support deprecated*)
   - Gas cost for the ``exp`` opcode increased, relevant for gas estimation and the optimizer.
- ``byzantium`` (*support deprecated*)
   - Opcodes ``returndatacopy``, ``returndatasize`` and ``staticcall`` are available in assembly.
   - The ``staticcall`` opcode is used when calling non-library view or pure functions, which prevents the functions from modifying state at the EVM level, i.e., even applies when you use invalid type conversions.
   - It is possible to access dynamic data returned from function calls.
   - ``revert`` opcode introduced, which means that ``revert()`` will not waste gas.
- ``constantinople``
   - Opcodes ``create2``, ``extcodehash``, ``shl``, ``shr`` and ``sar`` are available in assembly.
   - Shifting operators use shifting opcodes and thus need less gas.
- ``petersburg``
   - The compiler behaves the same way as with constantinople.
- ``istanbul``
   - Opcodes ``chainid`` and ``selfbalance`` are available in assembly.
- ``berlin``
   - Gas costs for ``SLOAD``, ``*CALL``, ``BALANCE``, ``EXT*`` and ``SELFDESTRUCT`` increased. The
     compiler assumes cold gas costs for such operations. This is relevant for gas estimation and
     the optimizer.
- ``london``
   - The block's base fee (`EIP-3198 <https://eips.ethereum.org/EIPS/eip-3198>`_ and `EIP-1559 <https://eips.ethereum.org/EIPS/eip-1559>`_) can be accessed via the global ``block.basefee`` or ``basefee()`` in inline assembly.
- ``paris``
   - Introduces ``prevrandao()`` and ``block.prevrandao``, and changes the semantics of the now deprecated ``block.difficulty``, disallowing ``difficulty()`` in inline assembly (see `EIP-4399 <https://eips.ethereum.org/EIPS/eip-4399>`_).
- ``shanghai`` (**default**)
   - Smaller code size and gas savings due to the introduction of ``push0`` (see `EIP-3855 <https://eips.ethereum.org/EIPS/eip-3855>`_).
- ``cancun``
   - The block's blob base fee (`EIP-7516 <https://eips.ethereum.org/EIPS/eip-7516>`_ and `EIP-4844 <https://eips.ethereum.org/EIPS/eip-4844>`_) can be accessed via the global ``block.blobbasefee`` or ``blobbasefee()`` in inline assembly.
   - Introduces ``blobhash()`` in inline assembly and a corresponding global function to retrieve versioned hashes of blobs associated with the transaction (see `EIP-4844 <https://eips.ethereum.org/EIPS/eip-4844>`_).
   - Opcode ``mcopy`` is available in assembly (see `EIP-5656 <https://eips.ethereum.org/EIPS/eip-5656>`_).
   - Opcodes ``tstore`` and ``tload`` are available in assembly (see `EIP-1153 <https://eips.ethereum.org/EIPS/eip-1153>`_).

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
      // Required: Source code language. Currently supported are "Solidity", "Yul", "SolidityAST" (experimental), "EVMAssembly" (experimental).
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
            // If files are used, their directories should be added to the command-line via
            // `--allow-paths <path>`.
          ]
        },
        "destructible":
        {
          // Optional: keccak256 hash of the source file
          "keccak256": "0x234...",
          // Required (unless "urls" is used): literal contents of the source file
          "content": "contract destructible is owned { function shutdown() { if (msg.sender == owner) selfdestruct(owner); } }"
        },
        "myFile.sol_json.ast":
        {
          // If language is set to "SolidityAST", an AST needs to be supplied under the "ast" key
          // and there can be only one source file present.
          // The format is the same as used by the `ast` output.
          // Note that importing ASTs is experimental and in particular that:
          // - importing invalid ASTs can produce undefined results and
          // - no proper error reporting is available on invalid ASTs.
          // Furthermore, note that the AST import only consumes the fields of the AST as
          // produced by the compiler in "stopAfter": "parsing" mode and then re-performs
          // analysis, so any analysis-based annotations of the AST are ignored upon import.
          "ast": { ... }
        },
        "myFile_evm.json":
        {
          // If language is set to "EVMAssembly", an EVM Assembly JSON object needs to be supplied
          // under the "assemblyJson" key and there can be only one source file present.
          // The format is the same as used by the `evm.legacyAssembly` output or `--asm-json`
          // output on the command line.
          // Note that importing EVM assembly is experimental.
          "assemblyJson":
          {
            ".code": [ ... ],
            ".data": { ... }, // optional
            "sourceList": [ ... ] // optional (if no `source` node was defined in any `.code` object)
          }
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
            // The inliner is always off if no details are given,
            // use details to switch it on.
            "inliner": false,
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
            // Use unchecked arithmetic when incrementing the counter of for loops
            // under certain circumstances. It is always on if no details are given.
            "simpleCounterForLoopUncheckedIncrement": true,
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
              // Select optimization steps to be applied. It is also possible to modify both the
              // optimization sequence and the clean-up sequence. Instructions for each sequence
              // are separated with the ":" delimiter and the values are provided in the form of
              // optimization-sequence:clean-up-sequence. For more information see
              // "The Optimizer > Selecting Optimizations".
              // This field is optional, and if not provided, the default sequences for both
              // optimization and clean-up are used. If only one of the sequences is provided
              // the other will not be run.
              // If only the delimiter ":" is provided then neither the optimization nor the clean-up
              // sequence will be run.
              // If set to an empty value, only the default clean-up sequence is used and
              // no optimization steps are applied.
              "optimizerSteps": "dhfoDgvulfnTUtnIf..."
            }
          }
        },
        // Version of the EVM to compile for.
        // Affects type checking and code generation. Can be homestead,
        // tangerineWhistle, spuriousDragon, byzantium, constantinople,
        // petersburg, istanbul, berlin, london, paris or shanghai (default)
        "evmVersion": "shanghai",
        // Optional: Change compilation pipeline to go through the Yul intermediate representation.
        // This is false by default.
        "viaIR": true,
        // Optional: Debugging settings
        "debug": {
          // How to treat revert (and require) reason strings. Settings are
          // "default", "strip", "debug" and "verboseDebug".
          // "default" does not inject compiler-generated revert strings and keeps user-supplied ones.
          // "strip" removes all revert strings (if possible, i.e. if literals are used) keeping side-effects
          // "debug" injects strings for compiler-generated internal reverts, implemented for ABI encoders V1 and V2 for now.
          // "verboseDebug" even appends further information to user-supplied revert strings (not yet implemented)
          "revertStrings": "default",
          // Optional: How much extra debug information to include in comments in the produced EVM
          // assembly and Yul code. Available components are:
          // - `location`: Annotations of the form `@src <index>:<start>:<end>` indicating the
          //    location of the corresponding element in the original Solidity file, where:
          //     - `<index>` is the file index matching the `@use-src` annotation,
          //     - `<start>` is the index of the first byte at that location,
          //     - `<end>` is the index of the first byte after that location.
          // - `snippet`: A single-line code snippet from the location indicated by `@src`.
          //     The snippet is quoted and follows the corresponding `@src` annotation.
          // - `*`: Wildcard value that can be used to request everything.
          "debugInfo": ["location", "snippet"]
        },
        // Metadata settings (optional)
        "metadata": {
          // The CBOR metadata is appended at the end of the bytecode by default.
          // Setting this to false omits the metadata from the runtime and deploy time code.
          "appendCBOR": true,
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
          // The top level key is the name of the source file where the library is used.
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
        //   irAst - AST of Yul intermediate representation of the code before optimization
        //   irOptimized - Intermediate representation after optimization
        //   irOptimizedAst - AST of intermediate representation after optimization
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
        //
        // Note that using `evm`, `evm.bytecode`, etc. will select every
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
          // Choose how division and modulo operations should be encoded.
          // When using `false` they are replaced by multiplication with slack
          // variables. This is the default.
          // Using `true` here is recommended if you are using the CHC engine
          // and not using Spacer as the Horn solver (using Eldarica, for example).
          // See the Formal Verification section for a more detailed explanation of this option.
          "divModNoSlacks": false,
          // Choose which model checker engine to use: all (default), bmc, chc, none.
          "engine": "chc",
          // Choose whether external calls should be considered trusted in case the
          // code of the called function is available at compile-time.
          // For details see the SMTChecker section.
          "extCalls": "trusted",
          // Choose which types of invariants should be reported to the user: contract, reentrancy.
          "invariants": ["contract", "reentrancy"],
          // Choose whether to output all proved targets. The default is `false`.
          "showProved": true,
          // Choose whether to output all unproved targets. The default is `false`.
          "showUnproved": true,
          // Choose whether to output all unsupported language features. The default is `false`.
          "showUnsupported": true,
          // Choose which solvers should be used, if available.
          // See the Formal Verification section for the solvers description.
          "solvers": ["cvc4", "smtlib2", "z3"],
          // Choose which targets should be checked: constantCondition,
          // underflow, overflow, divByZero, balance, assert, popEmptyArray, outOfBounds.
          // If the option is not given all targets are checked by default,
          // except underflow/overflow for Solidity >=0.8.7.
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
      // Optional: not present if no errors/warnings/infos were encountered
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
          // Mandatory: Component where the error originated, such as "general" etc.
          "component": "general",
          // Mandatory ("error", "warning" or "info", but please note that this may be extended in the future)
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
            // Intermediate representation before optimization (string)
            "ir": "",
            // AST of intermediate representation before optimization
            "irAst":  {/* ... */},
            // Intermediate representation after optimization (string)
            "irOptimized": "",
            // AST of intermediate representation after optimization
            "irOptimizedAst": {/* ... */},
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
13. ``YulException``: Error during Yul code generation - this should be reported as an issue.
14. ``Warning``: A warning, which didn't stop the compilation, but should be addressed if possible.
15. ``Info``: Information that the compiler thinks the user might find useful, but is not dangerous and does not necessarily need to be addressed.
