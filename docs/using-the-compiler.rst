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

If your contracts use :ref:`libraries <libraries>`, you will notice that the bytecode contains substrings of the form ``__$53aea86b7d70b31448b230b20ae141a537$__`` `(format was different <v0.5.0) <https://docs.soliditylang.org/en/v0.4.26/contracts.html#libraries>`_. These are placeholders for the actual library addresses.
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
- ``constantinople`` (*support deprecated*)
   - Opcodes ``create2``, ``extcodehash``, ``shl``, ``shr`` and ``sar`` are available in assembly.
   - Shifting operators use shifting opcodes and thus need less gas.
- ``petersburg`` (*support deprecated*)
   - The compiler behaves the same way as with constantinople.
- ``istanbul`` (*support deprecated*)
   - Opcodes ``chainid`` and ``selfbalance`` are available in assembly.
- ``berlin`` (*support deprecated*)
   - Gas costs for ``SLOAD``, ``*CALL``, ``BALANCE``, ``EXT*`` and ``SELFDESTRUCT`` increased. The
     compiler assumes cold gas costs for such operations. This is relevant for gas estimation and
     the optimizer.
- ``london``
   - The block's base fee (`EIP-3198 <https://eips.ethereum.org/EIPS/eip-3198>`_ and `EIP-1559 <https://eips.ethereum.org/EIPS/eip-1559>`_) can be accessed via the global ``block.basefee`` or ``basefee()`` in inline assembly.
- ``paris``
   - Introduces ``prevrandao()`` and ``block.prevrandao``, and changes the semantics of the now deprecated ``block.difficulty``, disallowing ``difficulty()`` in inline assembly (see `EIP-4399 <https://eips.ethereum.org/EIPS/eip-4399>`_).
- ``shanghai``
   - Smaller code size and gas savings due to the introduction of ``push0`` (see `EIP-3855 <https://eips.ethereum.org/EIPS/eip-3855>`_).
- ``cancun``
   - The block's blob base fee (`EIP-7516 <https://eips.ethereum.org/EIPS/eip-7516>`_ and `EIP-4844 <https://eips.ethereum.org/EIPS/eip-4844>`_) can be accessed via the global ``block.blobbasefee`` or ``blobbasefee()`` in inline assembly.
   - Introduces ``blobhash()`` in inline assembly and a corresponding global function to retrieve versioned hashes of blobs associated with the transaction (see `EIP-4844 <https://eips.ethereum.org/EIPS/eip-4844>`_).
   - Opcode ``mcopy`` is available in assembly (see `EIP-5656 <https://eips.ethereum.org/EIPS/eip-5656>`_).
   - Opcodes ``tstore`` and ``tload`` are available in assembly (see `EIP-1153 <https://eips.ethereum.org/EIPS/eip-1153>`_).
- ``prague``
- ``osaka`` (**default**)
   - ``clz`` builtin function is available in inline assembly. (`EIP-7939 <https://eips.ethereum.org/EIPS/eip-7939>`_)
- ``amsterdam`` (**experimental**)
   - The beacon chain slot number (`EIP-7843 <https://eips.ethereum.org/EIPS/eip-7843>`_) can be accessed via the global ``block.slotnum`` or ``slotnum()`` in inline assembly.

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
        "settable":
        {
          // Optional: keccak256 hash of the source file
          "keccak256": "0x234...",
          // Required (unless "urls" is used): literal contents of the source file
          "content": "contract settable is owned { uint256 private x = 0; function set(uint256 _x) public { if (msg.sender == owner) x = _x; } }"
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
        // Optional: List of remappings
        "remappings": [ ":g=/dir" ],
        // Optional: Experimental mode toggle (Default: false)
        // Makes it possible to use experimental features (but does not enable any such feature by itself).
        // The use of this mode is recorded in contract metadata.
        "experimental": true,
        // Optional: Optimizer settings
        "optimizer": {
          // Turn on the optimizer. Optional. Default: false.
          // NOTE: The state of the optimizer is fully determined by the 'details' dict and this setting
          // only affects its defaults - when enabled, all components default to being enabled.
          // The opposite is not true - there are several components that always default to being
          // enabled an can only be explicitly disabled via 'details'.
          // WARNING: Before version 0.8.6 omitting this setting was not equivalent to setting
          // it to false and would result in all components being disabled instead.
          // WARNING: Enabling optimizations for EVMAssembly input is allowed but not necessary under normal
          // circumstances. It forces the opcode-based optimizer to run again and can produce bytecode that
          // is not reproducible from metadata.
          "enabled": true,
          // Optimize for how many times you intend to run the code. Optional. Default: 200.
          // Lower values will optimize more for initial deployment cost, higher
          // values will optimize more for high-frequency usage.
          "runs": 200,
          // State of all optimizer components. Optional.
          // Default values are determined by whether the optimizer is enabled or not.
          // Note that the 'enabled' setting only affects the defaults here and has no effect when
          // all values are provided explicitly.
          "details": {
            // Peephole optimizer (opcode-based). Optional. Default: true.
            // Default for EVMAssembly input: false when optimization is not enabled.
            // NOTE: Always runs (even with optimization disabled) except for EVMAssembly input or when explicitly turned off here.
            "peephole": true,
            // Inliner (opcode-based). Optional. Default: true when optimization is enabled.
            "inliner": false,
            // Unused JUMPDEST remover (opcode-based). Optional. Default: true.
            // Default for EVMAssembly input: false when optimization is not enabled.
            // NOTE: Always runs (even with optimization disabled) except for EVMAssembly input or when explicitly turned off here.
            "jumpdestRemover": true,
            // Literal reordering (codegen-based). Optional. Default: true when optimization is enabled.
            // Moves literals to the right of commutative binary operators during code generation, helping exploit associativity.
            "orderLiterals": false,
            // Block deduplicator (opcode-based). Optional. Default: true when optimization is enabled.
            // Unifies assembly code blocks that share content.
            "deduplicate": false,
            // Common subexpression elimination (opcode-based). Optional. Default: true when optimization is enabled.
            // This is the most complicated step but can also provide the largest gain.
            "cse": false,
            // Constant optimizer (opcode-based). Optional. Default: true when optimization is enabled.
            // Tries to find better representations of literal numbers and strings, that satisfy the
            // size/cost trade-off determined by the 'runs' setting.
            "constantOptimizer": false,
            // Unchecked loop increment (codegen-based). Optional. Default: true.
            // Use unchecked arithmetic when incrementing the counter of 'for' loops under certain circumstances.
            // NOTE: Always runs (even with optimization disabled) unless explicitly turned off here.
            "simpleCounterForLoopUncheckedIncrement": true,
            // Yul optimizer. Optional. Default: true when optimization is enabled.
            // Used to optimize the IR produced by the Yul IR-based pipeline as well as inline assembly
            // and utility Yul code generated by the compiler.
            // NOTE: Before Solidity 0.6.0 the default was false.
            "yul": false,
            // Tuning options for the Yul optimizer. Optional.
            "yulDetails": {
              // Improve allocation of stack slots for variables, can free up stack slots early.
              // Optional. Default: true if Yul optimizer is enabled.
              "stackAllocation": true,
              // Optimization step sequence.
              // The general form of the value is "<main sequence>:<cleanup sequence>".
              // The setting is optional and when omitted, default values are used for both sequences.
              // If the value does not contain the ':' delimiter, it is interpreted as the main
              // sequence and the default is used for the cleanup sequence.
              // To make one of the sequences empty, the delimiter must be present at the first or last position.
              // In particular if the whole value consists only of the delimiter, both sequences are empty.
              // Note that there are several hard-coded steps that always run, even when both sequences are empty.
              // For more information see "The Optimizer > Selecting Optimizations".
              "optimizerSteps": "dfDvulfnTUtnIf..."
            }
          }
        },
        // Version of the EVM to compile for (optional).
        // Affects type checking and code generation. Can be homestead,
        // tangerineWhistle, spuriousDragon, byzantium, constantinople,
        // petersburg, istanbul, berlin, london, paris, shanghai, cancun,
        // prague, osaka (default), amsterdam (experimental), or @future (experimental).
        "evmVersion": "osaka",
        // Optional: Change compilation pipeline to go through the Yul intermediate representation.
        // This is false by default.
        "viaIR": true,
        // Optional: Turn on SSA CFG-based code generation via the IR (experimental).
        // Implies viaIR: true. This is false by default.
        "viaSSACFG": false,
        // Optional: Debugging settings
        "debug": {
          // How to treat revert (and require) reason strings. Settings are
          // "default", "strip", "debug" and "verboseDebug".
          // "default" does not inject compiler-generated revert strings and keeps user-supplied ones.
          // "strip" removes all revert strings (if possible, i.e. if literals are used) keeping side-effects.
          // NOTE: "strip" does not remove custom errors.
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
          //     Depends on `location`; selecting `snippet` without it is an error.
          // - `ast-id`: Annotations of the form `@ast-id <id>` over elements that can be mapped back to a definition in the original Solidity file.
          //   `<id>` is a node ID in the Solidity AST ('ast' output).
          // - `ethdebug`: Ethdebug annotations (experimental). Depends on `ast-id`; selecting
          //   `ethdebug` without `ast-id` is an error. Requesting an ethdebug output does not
          //   change this selection; without `ethdebug` in it the `evm.bytecode.ethdebug` and
          //   `evm.deployedBytecode.ethdebug` outputs carry none of the semantic debug info
          //   this component adds.
          // - `*`: Wildcard value that can be used to request all non-experimental components.
          "debugInfo": ["location", "snippet", "ast-id", "ethdebug"]
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
        // To select all outputs the compiler can possibly generate, with the exclusion of
        // Yul intermediate representation outputs, use
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
        //   irAst - AST of Yul intermediate representation of the code before optimization (experimental)
        //   irOptimized - Intermediate representation after optimization
        //   irOptimizedAst - AST of intermediate representation after optimization (experimental)
        //   storageLayout - Slots, offsets and types of the contract's state variables in storage
        //   transientStorageLayout - Slots, offsets and types of the contract's state variables in transient storage
        //   evm.assembly - New assembly format
        //   evm.legacyAssembly - Old-style assembly format in JSON
        //   evm.bytecode.ethdebug - Debug information in ethdebug format (ethdebug/format/program schema for creation bytecode). Can only be requested when compiling via IR. Carries semantic debug info only when the `ethdebug` component is present in `settings.debug.debugInfo`. (experimental)
        //   evm.deployedBytecode.ethdebug - Debug information in ethdebug format (ethdebug/format/program schema for deployed bytecode). Can only be requested when compiling via IR. Carries semantic debug info only when the `ethdebug` component is present in `settings.debug.debugInfo`. (experimental)
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
        //   yulCFGJson - Control Flow Graph (CFG) of the Single Static Assignment (SSA) form of the contract (experimental)
        //
        // Global level (needs "*" as file name and "*" as contract name):
        //   ethdebug.resources - Global ethdebug output (ethdebug/format/info/resources schema) containing source list and compiler info (experimental)
        //   ethdebug.compilation - Global ethdebug compilation output (the 'compilation' key from ethdebug/format/info/resources schema) (experimental)
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
          "showProvedSafe": true,
          // Choose whether to output all unproved targets. The default is `false`.
          "showUnproved": true,
          // Choose whether to output all unsupported language features. The default is `false`.
          "showUnsupported": true,
          // Choose which solvers should be used, if available.
          // See the Formal Verification section for the solvers description.
          "solvers": ["cvc5", "smtlib2", "z3"],
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
            // See the Storage Layout documentation.
            "transientStorageLayout": {"storage": [/* ... */], "types": {/* ... */} },
            // EVM-related outputs
            "evm": {
              // Assembly (string)
              "assembly": "",
              // Old-style assembly (object)
              "legacyAssembly": {},
              // Bytecode and related details.
              "bytecode": {
                // Ethdebug output (experimental)
                "ethdebug": {/* ... */},
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
                // Ethdebug output (experimental)
                "ethdebug": {/* ... */},
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
              },
              // Yul CFG representation of the SSA form (experimental)
              "yulCFGJson": {/* ... */}
            }
          }
        }
      },
      // Global Ethdebug output (experimental)
      "ethdebug": {
        // Requested via ethdebug.resources output selection
        "resources": {/* ... */},
        // Requested via ethdebug.compilation output selection
        "compilation": {/* ... */}
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

.. index:: ! Experimental mode, ! --experimental
.. _experimental-mode:

Experimental Mode
*****************

Some language and compiler features included in stable releases are not themselves considered stable.
They are sparsely documented, if at all, often not adequately tested, and thus not yet intended for production use.
In many cases it is possible to develop a big feature incrementally, with each iteration being already stable.
Sometimes, however, it is preferable to start with a prototype and stabilize it over multiple releases, while receiving feedback from users.
To prevent accidental use, such features can be only accessed by enabling the experimental mode.

There are no backwards compatibility guarantees for experimental features.
They are subject to change in breaking ways in non-breaking releases of the compiler.
Only major changes affecting them are recorded in the changelog.

To enable the experimental mode, use the ``--experimental`` flag on the command line,
or the analogous ``settings.experimental`` boolean setting in the Standard JSON input.

Note that the use of this mode is recorded in the metadata:

- ``experimental`` flag in CBOR metadata is set to ``true``,
- ``settings.experimental`` in JSON metadata is set to ``true``,

.. note::
    Prior to version 0.8.35, most of the experimental features were usable without any extra safeguards.
    Some were gated behind ``pragma experimental``, but this was not done consistently.
    The information about them was also only recorded in CBOR metadata and even then not always.
    The main goal of the experimental mode is to systematize this and make users fully aware when relying on features which are unfinished or not production-ready.

The table below details all currently available experimental features.

+-----------------------+--------------------------+------------------+-----------------------------------------------------------------------------------------------------------------------------------------+
| Feature               | ID                       | Affects bytecode | Flag/pragma                                                                                                                             |
+=======================+==========================+==================+=========================================================================================================================================+
| AST import            | ``ast-import``           | yes              | ``--import-ast``                                                                                                                        |
+-----------------------+--------------------------+------------------+-----------------------------------------------------------------------------------------------------------------------------------------+
| EVM Assembly import   | ``evmasm-import``        | yes              | ``--import-asm-json``                                                                                                                   |
+-----------------------+--------------------------+------------------+-----------------------------------------------------------------------------------------------------------------------------------------+
| IR AST                | ``ir-ast``               | no               | ``--ir-ast-json``, ``--ir-optimized-ast-json``                                                                                          |
+-----------------------+--------------------------+------------------+-----------------------------------------------------------------------------------------------------------------------------------------+
| Non-mainnet EVMs      | ``evm``                  | yes              | ``--evm-version <version name>``                                                                                                        |
+-----------------------+--------------------------+------------------+-----------------------------------------------------------------------------------------------------------------------------------------+
| Ethdebug              | ``ethdebug``             | no               | ``--ethdebug-resources``, ``--ethdebug-compilation``, ``--ethdebug-program``, ``--ethdebug-program-runtime``, ``--debug-info ethdebug`` |
+-----------------------+--------------------------+------------------+-----------------------------------------------------------------------------------------------------------------------------------------+
|                       |                          | no               | ``--yul-cfg-json``                                                                                                                      |
| SSA CFG               + ``ssa-cfg``              +------------------+-----------------------------------------------------------------------------------------------------------------------------------------+
|                       |                          | yes              | ``--via-ssa-cfg``                                                                                                                       |
+-----------------------+--------------------------+------------------+-----------------------------------------------------------------------------------------------------------------------------------------+
