### 0.8.22 (unreleased)

Language Features:
 * Allow defining events at file level.


Compiler Features:
 * Parser: Remove the experimental error recovery mode (``--error-recovery`` / ``settings.parserErrorRecovery``).
 * Yul Optimizer: If ``PUSH0`` is supported, favor zero literals over storing zero values in variables.
 * Yul Optimizer: Run the ``Rematerializer`` and ``UnusedPruner`` steps at the end of the default clean-up sequence.


Bugfixes:
 * AST: Fix wrong initial ID for Yul nodes in the AST.
 * NatSpec: Fix internal error when requesting userdoc or devdoc for a contract that emits an event defined in a foreign contract or interface.
 * SMTChecker: Fix encoding error that causes loops to unroll after completion.
 * SMTChecker: Fix inconsistency on constant condition checks when ``while`` or ``for`` loops are unrolled before the condition check.


### 0.8.21 (2023-07-19)

Important Bugfixes:
 * Code Generator: Always generate code for the expression in ``<expression>.selector`` in the legacy code generation pipeline.
 * Yul Optimizer: Fix ``FullInliner`` step (``i``) not preserving the evaluation order of arguments passed into inlined functions in code that is not in expression-split form (i.e. when using a custom optimizer sequence in which the step not preceded by ``ExpressionSplitter`` (``x``)).


Language Features:
 * Allow qualified access to events from other contracts.
 * Relax restrictions on initialization of immutable variables. Reads and writes may now happen at any point at construction time outside of functions and modifiers. Explicit initialization is no longer mandatory.


Compiler Features:
 * Commandline Interface: Add ``--ast-compact-json`` output in assembler mode.
 * Commandline Interface: Add ``--ir-ast-json`` and ``--ir-optimized-ast-json`` outputs for Solidity input, providing AST in compact JSON format for IR and optimized IR.
 * Commandline Interface: Respect ``--optimize-yul`` and ``--no-optimize-yul`` in compiler mode and accept them in assembler mode as well. ``--optimize --no-optimize-yul`` combination now allows enabling EVM assembly optimizer without enabling Yul optimizer.
 * EWasm: Remove EWasm backend.
 * Parser: Introduce ``pragma experimental solidity``, which will enable an experimental language mode that in particular has no stability guarantees between non-breaking releases and is not suited for production use.
 * SMTChecker: Add ``--model-checker-print-query`` CLI option and ``settings.modelChecker.printQuery`` JSON option to output the SMTChecker queries in the SMTLIB2 format. This requires using ``smtlib2`` solver only.
 * Standard JSON Interface: Add ``ast`` file-level output for Yul input.
 * Standard JSON Interface: Add ``irAst`` and ``irOptimizedAst`` contract-level outputs for Solidity input, providing AST in compact JSON format for IR and optimized IR.
 * Yul Optimizer: Remove experimental ``ReasoningBasedSimplifier`` optimization step.
 * Yul Optimizer: Stack-to-memory mover is now enabled by default whenever possible for via IR code generation and pure Yul compilation.


Bugfixes:
 * Code Generator: Disallow complex expressions whose results are types, built-ins, modules or some unassignable functions. The legacy code generation pipeline would not actually evaluate them, discarding any side-effects they might have.
 * Code Generator: Fix not entirely deterministic order of functions in unoptimized Yul output. The choice of C++ compiler in some cases would result in different (but equivalent) bytecode (especially from native binaries vs emscripten binaries).
 * Commandline Interface: Fix internal error when using ``--stop-after parsing`` and requesting some of the outputs that require full analysis or compilation.
 * Commandline Interface: It is no longer possible to specify both ``--optimize-yul`` and ``--no-optimize-yul`` at the same time.
 * SMTChecker: Fix encoding of side-effects inside ``if`` and ``ternary conditional``statements in the BMC engine.
 * SMTChecker: Fix false negative when a verification target can be violated only by trusted external call from another public function.
 * SMTChecker: Fix generation of invalid SMT-LIB2 scripts in BMC engine with trusted mode for external calls when CHC engine times out.
 * SMTChecker: Fix internal error caused by incorrectly classifying external function call using function pointer as a public getter.
 * SMTChecker: Fix internal error caused by using external identifier to encode member access to functions that take an internal function as a parameter.
 * Standard JSON Interface: Fix an incomplete AST being returned when analysis is interrupted by certain kinds of fatal errors.
 * Type Checker: Disallow using certain unassignable function types in complex expressions.
 * Type Checker: Function declaration types referring to different declarations are no longer convertible to each other.
 * Yul Optimizer: Ensure that the assignment of memory slots for variables moved to memory does not depend on AST IDs that may depend on whether additional files are included during compilation.
 * Yul Optimizer: Fix ``FullInliner`` step not ignoring code that is not in expression-split form.
 * Yul Optimizer: Fix optimized IR being unnecessarily passed through the Yul optimizer again before bytecode generation.


AST Changes:
 * AST: Add the ``experimentalSolidity`` field to the ``SourceUnit`` nodes, which indicate whether the experimental parsing mode has been enabled via ``pragma experimental solidity``.


### 0.8.20 (2023-05-10)

Compiler Features:
 * Assembler: Use ``push0`` for placing ``0`` on the stack for EVM versions starting from "Shanghai". This decreases the deployment and runtime costs.
 * EVM: Set default EVM version to "Shanghai".
 * EVM: Support for the EVM Version "Shanghai".
 * NatSpec: Add support for NatSpec documentation in ``enum`` definitions.
 * NatSpec: Add support for NatSpec documentation in ``struct`` definitions.
 * NatSpec: Include NatSpec from events that are emitted by a contract but defined outside of it in userdoc and devdoc output.
 * Optimizer: Re-implement simplified version of ``UnusedAssignEliminator`` and ``UnusedStoreEliminator``. It can correctly remove some unused assignments in deeply nested loops that were ignored by the old version.
 * Parser: Unary plus is no longer recognized as a unary operator in the AST and triggers an error at the parsing stage (rather than later during the analysis).
 * SMTChecker: Add CLI option ``--model-checker-bmc-loop-iterations`` and a JSON option ``settings.modelChecker.bmcLoopIterations`` that specify how many loop iterations the BMC engine should unroll. Note that false negatives are possible when unrolling loops. This is due to the possibility that bmc loop iteration setting is less than actual number of iterations needed to complete a loop.
 * SMTChecker: Group all messages about unsupported language features in a single warning. The CLI option ``--model-checker-show-unsupported`` and the JSON option ``settings.modelChecker.showUnsupported`` can be enabled to show the full list.
 * SMTChecker: Properties that are proved safe are now reported explicitly at the end of analysis. By default, only the number of safe properties is shown. The CLI option ``--model-checker-show-proved-safe`` and the JSON option ``settings.modelChecker.showProvedSafe`` can be enabled to show the full list of safe properties.
 * Standard JSON Interface: Add experimental support for importing ASTs via Standard JSON.
 * Yul EVM Code Transform: If available, use ``push0`` instead of ``codesize`` to produce an arbitrary value on stack in order to create equal stack heights between branches.


Bugfixes:
 * ABI: Include events in the ABI that are emitted by a contract but defined outside of it.
 * Immutables: Disallow initialization of immutables in try/catch statements.
 * SMTChecker: Fix false positives in ternary operators that contain verification targets in its branches, directly or indirectly.


AST Changes:
 * AST: Add the ``internalFunctionIDs`` field to the AST nodes of contracts containing IDs of functions that may be called via the internal dispatch. The field is a map from function AST IDs to internal dispatch function IDs. These IDs are always generated, but they are only used in via-IR code generation.
 * AST: Add the ``usedEvents`` field to ``ContractDefinition`` which contains the AST IDs of all events emitted by the contract as well as all events defined and inherited by the contract.


### 0.8.19 (2023-02-22)

Language Features:
 * Allow defining custom operators for user-defined value types via ``using {f as +} for T global`` syntax.


Compiler Features:
 * SMTChecker: New trusted mode that assumes that any compile-time available code is the actual used code even in external calls. This can be used via the CLI option ``--model-checker-ext-calls trusted`` or the JSON field ``settings.modelChecker.extCalls: "trusted"``.


Bugfixes:
 * Assembler: Avoid duplicating subassembly bytecode where possible.
 * Code Generator: Avoid including references to the deployed label of referenced functions if they are called right away.
 * ContractLevelChecker: Properly distinguish the case of missing base constructor arguments from having an unimplemented base function.
 * SMTChecker: Fix internal error caused by unhandled ``z3`` expressions that come from the solver when bitwise operators are used.
 * SMTChecker: Fix internal error when using the custom NatSpec annotation to abstract free functions.
 * TypeChecker: Also allow external library functions in ``using for``.


AST Changes:
 * AST: Add ``function`` field to ``UnaryOperation`` and ``BinaryOperation`` AST nodes. ``functionList`` in ``UsingForDirective`` AST nodes will now contain ``operator`` and ``definition`` members instead of ``function`` when the list entry defines an operator.


### 0.8.18 (2023-02-01)

Language Features:
 * Allow named parameters in mapping types.


Compiler Features:
 * Commandline Interface: Add ``--no-cbor-metadata`` that skips CBOR metadata from getting appended at the end of the bytecode.
 * Commandline Interface: Return exit code ``2`` on uncaught exceptions.
 * EVM: Deprecate ``block.difficulty`` and disallow ``difficulty()`` in inline assembly for EVM versions >= paris. The change is due to the renaming introduced by [EIP-4399](https://eips.ethereum.org/EIPS/eip-4399).
 * EVM: Introduce ``block.prevrandao`` in Solidity and ``prevrandao()`` in inline assembly for EVM versions >= paris.
 * EVM: Set the default EVM version to "Paris".
 * EVM: Support for the EVM version "Paris".
 * Language Server: Add basic document hover support.
 * Natspec: Add event Natspec inheritance for devdoc.
 * Optimizer: Added optimization rule ``and(shl(X, Y), shl(X, Z)) => shl(X, and(Y, Z))``.
 * Parser: More detailed error messages about invalid version pragmas.
 * SMTChecker: Make ``z3`` the default solver for the BMC and CHC engines instead of all solvers.
 * SMTChecker: Support Eldarica as a Horn solver for the CHC engine when using the CLI option ``--model-checker-solvers eld``. The binary ``eld`` must be available in the system.
 * Solidity Upgrade Tool: Remove ``solidity-upgrade`` tool.
 * Standard JSON: Add a boolean field ``settings.metadata.appendCBOR`` that skips CBOR metadata from getting appended at the end of the bytecode.
 * TypeChecker: Warn when using deprecated builtin ``selfdestruct``.
 * Yul EVM Code Transform: Generate more optimal code for user-defined functions that always terminate a transaction. No return labels will be pushed for calls to functions that always terminate.
 * Yul Optimizer: Allow replacing the previously hard-coded cleanup sequence by specifying custom steps after a colon delimiter (``:``) in the sequence string.
 * Yul Optimizer: Eliminate ``keccak256`` calls if the value was already calculated by a previous call and can be reused.


Bugfixes:
 * Parser: Disallow several ``indexed`` attributes for the same event parameter.
 * Parser: Disallow usage of the ``indexed`` attribute for modifier parameters.
 * SMTChecker: Fix display error for negative integers that are one more than powers of two.
 * SMTChecker: Fix internal error on chain assignments using static fully specified state variables.
 * SMTChecker: Fix internal error on multiple wrong SMTChecker natspec entries.
 * SMTChecker: Fix internal error when a public library function is called internally.
 * SMTChecker: Fix internal error when deleting struct member of function type.
 * SMTChecker: Fix internal error when using user-defined types as mapping indices or struct members.
 * SMTChecker: Improved readability for large integers that are powers of two or almost powers of two in error messages.
 * TypeChecker: Fix bug where private library functions could be attached with ``using for`` outside of their declaration scope.
 * Yul Optimizer: Hash hex and decimal literals according to their value instead of their representation, improving the detection of equivalent functions.


### 0.8.17 (2022-09-08)

Important Bugfixes:
 * Yul Optimizer: Prevent the incorrect removal of storage writes before calls to Yul functions that conditionally terminate the external EVM call.


Compiler Features:
 * Code Generator: More efficient overflow checks for multiplication.
 * Language Server: Analyze all files in a project by default (can be customized by setting ``'file-load-strategy'`` to ``'directly-opened-and-on-import'`` in LSP settings object).
 * Yul Optimizer: Simplify the starting offset of zero-length operations to zero.


Bugfixes:
 * Type Checker: Fix internal compiler error on tuple assignments with invalid left-hand side.
 * Yul IR Code Generation: Fix internal compiler error when accessing the ``.slot`` member of a mapping through a storage reference in inline assembly.


Build System:
 * Allow disabling pedantic warnings and do not treat warnings as errors during compiler build when ``-DPEDANTIC=OFF`` flag is passed to CMake.
 * Update emscripten to version 3.1.19.


### 0.8.16 (2022-08-08)

Important Bugfixes:
 * Code Generation: Fix data corruption that affected ABI-encoding of calldata values represented by tuples: structs at any nesting level; argument lists of external functions, events and errors; return value lists of external functions. The 32 leading bytes of the first dynamically-encoded value in the tuple would get zeroed when the last component contained a statically-encoded array.


Compiler Features:
 * Code Generator: More efficient code for checked addition and subtraction.
 * TypeChecker: Support using library constants in initializers of other constants.
 * Yul IR Code Generation: Improved copy routines for arrays with packed storage layout.
 * Yul Optimizer: Add rule to convert ``mod(add(X, Y), A)`` into ``addmod(X, Y, A)``, if ``A`` is a power of two.
 * Yul Optimizer: Add rule to convert ``mod(mul(X, Y), A)`` into ``mulmod(X, Y, A)``, if ``A`` is a power of two.


Bugfixes:
 * Commandline Interface: Disallow the following options outside of the compiler mode: ``--via-ir``,``--metadata-literal``, ``--metadata-hash``, ``--model-checker-show-unproved``, ``--model-checker-div-mod-no-slacks``, ``--model-checker-engine``, ``--model-checker-invariants``, ``--model-checker-solvers``, ``--model-checker-timeout``, ``--model-checker-contracts``, ``--model-checker-targets``.
 * Type Checker: Fix compiler crash on tuple assignments involving certain patterns with unary tuples on the left-hand side.
 * Type Checker: Fix compiler crash when ``abi.encodeCall`` received a tuple expression instead of an inline tuple.
 * Type Checker: Fix null dereference in ``abi.encodeCall`` type checking of free function.


### 0.8.15 (2022-06-15)

Important Bugfixes:
 * Code Generation: Avoid writing dirty bytes to storage when copying ``bytes`` arrays.
 * Yul Optimizer: Keep all memory side-effects of inline assembly blocks.


Language Features:
 * Add `E.selector` for a non-anonymous event `E` to access the 32-byte selector topic.


Compiler Features:
 * Language Server: Add rudimentary support for semantic highlighting.
 * Language Server: Adds support for configuring ``include-paths`` JSON settings object that can be passed during LSP configuration stage.
 * Language Server: Always add ``{project_root}/node_modules`` to include search paths.
 * Type Checker: Warn about assignments involving multiple pushes to storage ``bytes`` that may invalidate references.
 * Yul Optimizer: Improve inlining heuristics for via IR code generation and pure Yul compilation.

Bugfixes:
 * ABI Encoder: When encoding an empty string coming from storage do not add a superfluous empty slot for data.
 * Common Subexpression Eliminator: Process assembly items in chunks with maximum size of 2000. It helps to avoid extremely time-consuming searches during code optimization.
 * DocString Parser: Fix ICE caused by an immutable struct with mapping.
 * Yul IR Code Generation: More robust cleanup in corner cases during memory to storage copies.
 * Yul Optimizer: Do not remove ``returndatacopy`` in cases in which it might perform out-of-bounds reads that unconditionally revert as out-of-gas. Previously, any ``returndatacopy`` that wrote to memory that was never read from was removed without accounting for the out-of-bounds condition.


### 0.8.14 (2022-05-17)

Important Bugfixes:
 * ABI Encoder: When ABI-encoding values from calldata that contain nested arrays, correctly validate the nested array length against ``calldatasize()`` in all cases.
 * Override Checker: Allow changing data location for parameters only when overriding external functions.


Compiler Features:
 * Assembly-Json Exporter: Include source list in `sourceList` field.
 * Commandline Interface: Option ``--pretty-json`` works also with the following options: ``--abi``, ``--asm-json``, ``--ast-compact-json``, ``--devdoc``, ``--storage-layout``, ``--userdoc``.
 * Language Server: Allow full filesystem access to language server.
 * Peephole Optimizer: Remove operations without side effects before simple terminations.
 * SMTChecker: Support ``abi.encodeCall`` taking into account the called selector.


Bugfixes:
 * Assembly-Json Exporter: Fix assembly json export to store jump types of operations in `jumpType` field instead of `value`.
 * SMTChecker: Fix ABI compatibility with z3 >=4.8.16.
 * SMTChecker: Fix bug when z3 is selected but not available at runtime.
 * Type Checker: Properly check restrictions of ``using ... global`` in conjunction with libraries.
 * TypeChecker: Convert parameters of function type to how they would be called for ``abi.encodeCall``.


### 0.8.13 (2022-03-16)

Important Bugfixes:
 * Code Generator: Correctly encode literals used in ``abi.encodeCall`` in place of fixed bytes arguments.


Language Features:
 * General: Allow annotating inline assembly as memory-safe to allow optimizations and stack limit evasion that rely on respecting Solidity's memory model.
 * General: ``using M for Type;`` is allowed at file level and ``M`` can now also be a brace-enclosed list of free functions or library functions.
 * General: ``using ... for T global;`` is allowed at file level where the user-defined type ``T`` has been defined, resulting in the effect of the statement being available everywhere ``T`` is available.


Compiler Features:
 * Commandline Interface: Allow the use of ``--via-ir`` in place of ``--experimental-via-ir``.
 * Compilation via Yul IR is no longer marked as experimental.
 * JSON-AST: Added selector field for errors and events.
 * Language Server: Implements goto-definition.
 * Peephole Optimizer: Optimize comparisons in front of conditional jumps and conditional jumps across a single unconditional jump.
 * Yul EVM Code Transform: Avoid unnecessary ``pop``s on terminating control flow.
 * Yul IR Code Generation: When the result of an external call is statically-sized, ignore any returndata past the size expected by the compiler.
 * Yul Optimizer: Remove ``sstore`` and ``mstore`` operations that are never read from.


Bugfixes:
 * General: Fix internal error for locales with unusual capitalization rules. Locale set in the environment is now completely ignored.
 * Type Checker: Fix incorrect type checker errors when importing overloaded functions.
 * Yul IR Code Generation: Optimize embedded creation code with correct settings. This fixes potential mismatches between the constructor code of a contract compiled in isolation and the bytecode in ``type(C).creationCode``, resp. the bytecode used for ``new C(...)``.


### 0.8.12 (2022-02-16)

Language Features:
 * General: Add equality-comparison operators for external function types.
 * General: Support ``ContractName.functionName`` for ``abi.encodeCall``, in addition to external function pointers.


Compiler Features:
 * Commandline Interface: Event and error signatures are also returned when using ``--hashes``.
 * Yul Optimizer: Remove ``mstore`` and ``sstore`` operations if the slot already contains the same value.
 * Yul: Emit immutable references for pure yul code when requested.



Bugfixes:
 * Antlr Grammar: Allow builtin names in ``yulPath`` to support ``.address`` in function pointers.
 * Code Generator: Fix internal error when accessing the members of external functions occupying more than two stack slots.
 * Code Generator: Fix internal error when doing an explicit conversion from ``string calldata`` to ``bytes``.
 * Control Flow Graph: Perform proper virtual lookup for modifiers for uninitialized variable and unreachable code analysis.
 * General: ``string.concat`` now properly takes strings as arguments and returns ``string memory``. It was accidentally introduced as a copy of ``bytes.concat`` before.
 * Immutables: Fix wrong error when the constructor of a base contract uses ``return`` and the derived contract contains immutable variables.
 * Inheritance: Consider functions in all ancestors during override analysis.
 * IR Generator: Add missing cleanup during the conversion of fixed bytes types to smaller fixed bytes types.
 * IR Generator: Add missing cleanup for indexed event arguments of value type.
 * IR Generator: Fix internal error when copying reference types in calldata and storage to struct or array members in memory.
 * IR Generator: Fix IR syntax error when copying storage arrays of structs containing functions.
 * Natspec: Fix internal error when overriding a struct getter with a Natspec-documented return value and the name in the struct is different.
 * Type Checker: Fix internal error when a constant variable declaration forward references a struct.
 * Yul EVM Code Transform: Improved stack shuffling in corner cases.


Solc-Js:
 * The wrapper now requires at least nodejs v10.
 * The code has been ported to TypeScript.


Build System:
 * Emscripten builds store the embedded WebAssembly binary in LZ4 compressed format and transparently decompress on loading.


### 0.8.11 (2021-12-20)

Language Features:
 * General: New builtin function ``abi.encodeCall(functionPointer, (arg1, arg2, ...))`` that type-checks the arguments and returns the ABI-encoded function call data.


Compiler Features:
 * Commandline Interface: Add ``--lsp`` option to get ``solc`` to act as a Language Server (LSP) communicating over stdio.


Bugfixes:
 * Code Generator: Fix a crash when using ``@use-src`` and compiling from Yul to ewasm.
 * SMTChecker: Fix internal error when an unsafe target is solved more than once and the counterexample messages are different.
 * SMTChecker: Fix soundness of assigned storage/memory local pointers that were not erasing enough knowledge.
 * Fix internal error when a function has a calldata struct argument with an internal type inside.
 * IR Generator: Fix IR syntax error when copying storage arrays of functions.


### 0.8.10 (2021-11-09)

Language Features:
 * Inline Assembly: Support ``.address`` and ``.selector`` on external function pointers to access their address and function selector.


Compiler Features:
 * Code Generator: Skip existence check for external contract if return data is expected. In this case, the ABI decoder will revert if the contract does not exist.
 * Commandline Interface: Accept nested brackets in step sequences passed to ``--yul-optimizations``.
 * Commandline Interface: Add ``--debug-info`` option for selecting how much extra debug information should be included in the produced EVM assembly and Yul code.
 * Commandline Interface: Support ``--asm``, ``--bin``, ``--ir-optimized``, ``--ewasm`` and ``--ewasm-ir`` output selection options in assembler mode.
 * Commandline Interface: Use different colors when printing errors, warnings and infos.
 * JSON AST: Set absolute paths of imports earlier, in the ``parsing`` stage.
 * SMTChecker: Output values for ``block.*``, ``msg.*`` and ``tx.*`` variables that are present in the called functions.
 * SMTChecker: Report contract invariants and reentrancy properties. This can be enabled via the CLI option ``--model-checker-invariants`` or the Standard JSON option ``settings.modelChecker.invariants``.
 * Standard JSON: Accept nested brackets in step sequences passed to ``settings.optimizer.details.yulDetails.optimizerSteps``.
 * Standard JSON: Add ``settings.debug.debugInfo`` option for selecting how much extra debug information should be included in the produced EVM assembly and Yul code.
 * Yul EVM Code Transform: Switch to new optimized code transform when compiling via Yul with enabled optimizer.
 * Yul Optimizer: Take control-flow side-effects of user-defined functions into account in various optimizer steps.


Bugfixes:
 * Code Generator: Fix constructor source mappings for immutables.
 * Commandline Interface: Disallow ``--error-recovery`` option outside of the compiler mode.
 * Commandline Interface: Don't return zero exit code when writing linked files to disk fails.
 * Commandline Interface: Fix extra newline character being appended to sources passed through standard input, affecting their hashes.
 * Commandline Interface: Report output selection options unsupported by the selected input mode instead of ignoring them.
 * Commandline Interface: When linking only accept exact matches for library names passed to the ``--libraries`` option. Library names not prefixed with a file name used to match any library with that name.
 * SMTChecker: Fix internal error in magic type access (``block``, ``msg``, ``tx``).
 * SMTChecker: Fix internal error in the CHC engine when passing gas in the function options.
 * TypeChecker: Fix internal error when using arrays and structs with user defined value types before declaration.
 * TypeChecker: Fix internal error when using user defined value types in public library functions.
 * TypeChecker: Improved error message for constant variables with (nested) mapping types.
 * Yul Assembler: Fix internal error when function names are not unique.
 * Yul IR Generator: Do not output empty switches/if-bodies for empty contracts.


Important Bugfixes in Experimental Features:
 * Yul IR Generator: Changes to function return variables referenced in modifier invocation arguments were not properly forwarded if there was more than one return variable.


Build System:
 * Pass linker-only emscripten options only when linking.
 * Remove obsolete compatibility workaround for emscripten builds.
 * Update emscripten to version 2.0.33.


### 0.8.9 (2021-09-29)

Important Bugfixes:
 * Immutables: Properly perform sign extension on signed immutables.
 * User Defined Value Type: Fix storage layout of user defined value types for underlying types shorter than 32 bytes.


Bugfixes:
 * AST: Export ``canonicalName`` for ``UserDefinedValueTypeDefinition`` and ``ContractDefinition``.



### 0.8.8 (2021-09-27)

Language Features:
 * Inheritance: A function that overrides only a single interface function does not require the ``override`` specifier.
 * Type System: Support ``type(E).min`` and ``type(E).max`` for enums.
 * User Defined Value Type: allows creating a zero cost abstraction over a value type with stricter type requirements.


Compiler Features:
 * Commandline Interface: Add ``--include-path`` option for specifying extra directories that may contain importable code (e.g. packaged third-party libraries).
 * Commandline Interface: Do not implicitly run evm bytecode generation unless needed for the requested output.
 * Commandline Interface: Normalize paths specified on the command line and make them relative for files located inside base path and/or include paths.
 * Immutable variables can be read at construction time once they are initialized.
 * SMTChecker: Add constraints to better correlate ``address(this).balance`` and ``msg.value``.
 * SMTChecker: Support constants via modules.
 * SMTChecker: Support low level ``call`` as external calls to unknown code.
 * SMTChecker: Support the ``value`` option for external function calls.
 * SMTChecker: Support user defined value types.


Bugfixes:
 * Code Generator: Fix ICE on assigning to calldata structs and statically-sized calldata arrays in inline assembly.
 * Code Generator: Use stable source order for ABI functions.
 * Commandline Interface: Disallow the ``--experimental-via-ir`` option in Standard JSON, Assembler and Linker modes.
 * Commandline Interface: Fix resolution of paths whitelisted with ``--allowed-paths`` or implicitly due to base path, remappings and files being compiled. Correctly handle paths that do not match imports exactly due to being relative, non-normalized or empty.
 * Commandline Interface: Report optimizer options as invalid in Standard JSON and linker modes instead of ignoring them.
 * Name Resolver: Fix that when importing an aliased symbol using ``import {AliasedName} from "a.sol"`` it would use the original name of the symbol and not the aliased one.
 * Opcode Optimizer: Prevent the optimizer from running multiple times to avoid potential bytecode differences for referenced code.
 * Parser: Properly check for multiple SPDX license identifiers next to each other and validate them.
 * SMTChecker: Fix BMC's constraints regarding internal functions.
 * SMTChecker: Fix false negative caused by ``push`` on storage array references returned by internal functions.
 * SMTChecker: Fix false positive in external calls from constructors.
 * SMTChecker: Fix internal error on some multi-source uses of ``abi.*``, cryptographic functions and constants.
 * Standard JSON: Fix non-fatal errors in Yul mode being discarded if followed by a fatal error.
 * Type Checker: Correct wrong error message in inline assembly complaining about ``.slot`` or ``.offset`` not valid when actually ``.length`` was used.
 * Type Checker: Disallow modifier declarations and definitions in interfaces.
 * Yul Optimizer: Fix a crash in LoadResolver, when ``keccak256`` has particular non-identifier arguments.



### 0.8.7 (2021-08-11)

Language Features:
 * Introduce global ``block.basefee`` for retrieving the base fee of the current block.
 * Yul: Introduce builtin ``basefee()`` for retrieving the base fee of the current block.


Compiler Features:
 * AssemblyStack: Also run opcode-based optimizer when compiling Yul code.
 * Commandline Interface: option ``--pretty-json`` works also with ``--standard--json``.
 * EVM: Set the default EVM version to "London".
 * SMTChecker: Do not check underflow and overflow by default.
 * SMTChecker: Unproved targets are hidden by default, and the SMTChecker only states how many unproved targets there are. They can be listed using the command line option ``--model-checker-show-unproved`` or the JSON option ``settings.modelChecker.showUnproved``.
 * SMTChecker: new setting to enable/disable encoding of division and modulo with slack variables. The command line option is ``--model-checker-div-mod-slacks`` and the JSON option is ``settings.modelChecker.divModWithSlacks``.
 * Yul EVM Code Transform: Also pop unused argument slots for functions without return variables (under the same restrictions as for functions with return variables).
 * Yul EVM Code Transform: Do not reuse stack slots that immediately become unreachable.
 * Yul Optimizer: Move function arguments and return variables to memory with the experimental Stack Limit Evader (which is not enabled by default).


Bugfixes:
 * Code Generator: Fix crash when passing an empty string literal to ``bytes.concat()``.
 * Code Generator: Fix internal compiler error when calling functions bound to calldata structs and arrays.
 * Code Generator: Fix internal compiler error when passing a 32-byte hex literal or a zero literal to ``bytes.concat()`` by disallowing such literals.
 * Commandline Interface: Apply ``--optimizer-runs`` option in assembly / yul mode.
 * Commandline Interface: Fix crash when a directory path is passed to ``--standard-json``.
 * Commandline Interface: Read JSON from standard input when ``--standard-json`` gets ``-`` as a file name.
 * Standard JSON: Include source location for errors in files with empty name.
 * Type Checker: Fix internal error and prevent static calls to unimplemented modifiers.
 * Yul Code Generator: Fix internal compiler error when using a long literal with bitwise negation.
 * Yul Code Generator: Fix source location references for calls to builtin functions.
 * Yul Parser: Fix source location references for ``if`` statements.


### 0.8.6 (2021-06-22)

Language Features:
 * Yul: Special meaning of ``".metadata"`` data object in Yul object.


Bugfixes:
 * Control Flow Graph: Fix incorrectly reported unreachable code.
 * Solc-Js: When running ``solcjs`` without the ``--optimize`` flag, use ``settings.optimizer.enabled=false`` in Standard JSON instead of omitting the key.
 * Standard JSON: Omitting ``settings.optimizer.enabled`` was not equivalent to setting it to ``false``. It meant disabling also the peephole optimizer and jumpdest remover which by default still run with ``enabled=false``.


### 0.8.5 (2021-06-10)

Language Features:
 * Allowing conversion from ``bytes`` and ``bytes`` slices to ``bytes1``/.../``bytes32``.
 * Yul: Add ``verbatim`` builtin function to inject arbitrary bytecode.


Compiler Features:
 * Code Generator: Insert helper functions for panic codes instead of inlining unconditionally. This can reduce costs if many panics (checks) are inserted, but can increase costs where few panics are used.
 * EVM: Set the default EVM version to "Berlin".
 * SMTChecker: Function definitions can be annotated with the custom Natspec tag ``custom:smtchecker abstract-function-nondet`` to be abstracted by a nondeterministic value when called.
 * Standard JSON / combined JSON: New artifact "functionDebugData" that contains bytecode offsets of entry points of functions and potentially more information in the future.
 * Yul Optimizer: Evaluate ``keccak256(a, c)``, when the value at memory location ``a`` is known at compile time and ``c`` is a constant ``<= 32``.


Bugfixes:
 * AST: Do not output value of Yul literal if it is not a valid UTF-8 string.
 * Code Generator: Fix internal error when function arrays are assigned to storage variables and the function types can be implicitly converted but are not identical.
 * Code Generator: Fix internal error when super would have to skip an unimplemented function in the virtual resolution order.
 * Control Flow Graph: Assume unimplemented modifiers use a placeholder.
 * Control Flow Graph: Take internal calls to functions that always revert into account for reporting unused or unassigned variables.
 * Function Call Graph: Fix internal error connected with circular constant references.
 * Name Resolver: Do not issue shadowing warning if the shadowing name is not directly accessible.
 * Natspec: Allow multiple ``@return`` tags on public state variable documentation.
 * SMTChecker: Fix internal error on conversion from ``bytes`` to ``fixed bytes``.
 * SMTChecker: Fix internal error on external calls from the constructor.
 * SMTChecker: Fix internal error on struct constructor with fixed bytes member initialized with string literal.
 * Source Locations: Properly set source location of scoped blocks.
 * Standard JSON: Properly allow the ``inliner`` setting under ``settings.optimizer.details``.
 * Type Checker: Fix internal compiler error related to having mapping types in constructor parameter for abstract contracts.
 * Type Checker: Fix internal compiler error when attempting to use an invalid external function type on pre-byzantium EVMs.
 * Type Checker: Fix internal compiler error when overriding receive ether function with one having different parameters during inheritance.
 * Type Checker: Make errors about (nested) mapping type in event or error parameter into fatal type errors.
 * Type Checker: Fix internal compiler error when overriding an implemented modifier with an unimplemented one.


AST Changes:
 * Add member `hexValue` for Yul string and hex literals.



### 0.8.4 (2021-04-21)

Important Bugfixes:
 * ABI Decoder V2: For two-dimensional arrays and specially crafted data in memory, the result of ``abi.decode`` can depend on data elsewhere in memory. Calldata decoding is not affected.


Language Features:
 * Assembly / Yul: Allow hex string literals.
 * Possibility to use ``bytes.concat`` with variable number of ``bytes`` and ``bytesNN`` arguments which behaves as a restricted version of `abi.encodePacked` with a more descriptive name.
 * Support custom errors via the ``error`` keyword and introduce the ``revert`` statement.


Compiler Features:
 * Analysis: Properly detect circular references to the bytecode of other contracts across all function calls.
 * Commandline Interface: Model checker option ``--model-checker-targets`` also accepts ``outOfBounds``.
 * Commandline Interface: New model checker option ``--model-checker-contracts`` allows users to select which contracts should be analyzed as the most derived.
 * Low-Level Inliner: Inline ordinary jumps to small blocks and jumps to small blocks that terminate.
 * NatSpec: Allow ``@notice`` tag on non-public state variables and local variable declarations. The documentation will only be part of the AST, under the field ``documentation``.
 * SMTChecker: Deprecate ``pragma experimental SMTChecker;`` and set default model checker engine to ``none``.
 * SMTChecker: Report local variables in CHC counterexamples.
 * SMTChecker: Report out of bounds index access for arrays and fixed bytes.
 * SMTChecker: Support file level functions and constants.
 * Standard JSON: Model checker option ``settings.modelChecker.targets`` also accepts ``outOfBounds``.
 * Standard JSON: Model checker option ``settings.modelChecker.targets`` takes an array of string targets instead of string of comma separated targets.
 * Standard JSON: New model checker option ``settings.modelChecker.contracts`` allows users to select which contracts should be analyzed as the most derived.
 * Yul EVM Code Transform: Stack Optimization: Reuse slots of unused function arguments and defer allocating stack slots for return variables until after expression statements and assignments that do not reference them.
 * Yul Optimizer: Added a new step FunctionSpecializer, that specializes a function with its literal arguments.


Bugfixes:
 * Antlr Grammar: Fix parsing of import paths involving properly distinguishing between empty and non-empty string literals in general.
 * AST Output: Fix ``kind`` field of ``ModifierInvocation`` for base constructor calls.
 * Commandline interface: Fix internal error when printing AST and using ``--base-path`` or ``file://`` prefix in imports.
 * Commandline interface: Fix standard input bypassing allowed path checks.
 * Natspec: Fix internal error related to the `@returns` documentation for a public state variable overriding a function.
 * SMTChecker: Fix false positive and false negative on ``push`` as LHS of a compound assignment.
 * SMTChecker: Fix false positive in contracts that cannot be deployed.
 * SMTChecker: Fix internal error on public getter returning dynamic data on older EVM versions where these are not available.
 * SMTChecker: Fix internal error on try-catch with function call in catch block.
 * Type Checker: Fix missing error when events are used without an emit statement.


AST Changes:
 * New property for ``ContractDefinition`` nodes: ``usedErrors`` lists AST IDs of all errors used by the contract (even if defined outside).



### 0.8.3 (2021-03-23)

Important Bugfixes:
 * Optimizer: Fix bug on incorrect caching of Keccak-256 hashes.

Compiler Features:
 * Command Line Interface: Drop experimental support for ``--machine evm15``.
 * Optimizer: Try to move ``and`` with constant inside ``or`` to improve storage writes of small types.
 * Optimizer: Replace multiplications and divisions with powers of two by shifts.

Bugfixes:
 * AST Import: For constructors, a public visibility is ignored during importing.
 * Error Reporter: Fix handling of carriage return.
 * SMTChecker: Fix internal error in BMC on resolving virtual functions inside branches.
 * SMTChecker: Fix internal error on ``array.pop`` nested inside 1-tuple.
 * SMTChecker: Fix internal error on ``FixedBytes`` constant initialized with string literal.
 * SMTChecker: Fix internal error on array slices.
 * SMTChecker: Fix internal error on calling public getter on a state variable of type array (possibly nested) of structs.
 * SMTChecker: Fix internal error on pushing to ``string`` casted to ``bytes``.
 * SMTChecker: Fix bug in virtual functions called by constructors.

AST Changes:
 * ModifierInvocation: Add ``kind`` field which can be ``modifierInvocation`` or ``baseConstructorSpecifier``.


### 0.8.2 (2021-03-02)

Compiler Features:
 * AST: Export NatSpec comments above each statement as their documentation.
 * Inline Assembly: Do not warn anymore about variables or functions being shadowed by EVM opcodes.
 * NatSpec: Provide source locations for parsing errors.
 * Optimizer: Simple inlining when jumping to small blocks that jump again after a few side-effect free opcodes.
 * NatSpec: Allow and export all tags that start with ``@custom:``.


Bugfixes:
 * AST: Added ``referencedDeclaration`` for enum members.
 * Code Generator: Fix internal error when functions are passed as parameters of other callables, when the function types can be implicitly converted, but not identical.
 * Parser: Properly parse ``.address`` in some situations.
 * SMTChecker: Fix missing type constraints on block and transaction variables in the deployment phase.
 * Type Checker: Fix internal error when override specifier is not a contract.
 * Type Checker: Make function-hash collision errors into fatal type errors.


AST Changes:
 * Adds ``nameLocation`` to declarations to represent the exact location of the symbolic name.
 * Removed the redundant function type "bytearraypush" - replaced by "arraypush".
 * Support field ``documentation`` to hold NatSpec comments above each statement.


### 0.8.1 (2021-01-27)

Language Features:
 * Possibility to use ``catch Panic(uint code)`` to catch a panic failure from an external call.

Compiler Features:
 * Code Generator: Reduce the cost of ``<address>.code.length`` by using ``extcodesize`` directly.
 * Command Line Interface: Allow ``=`` as separator between library name and address in ``--libraries`` commandline option.
 * Command Line Interface: New option ``--model-checker-targets`` allows specifying which targets should be checked. The valid options are ``all``, ``constantCondition``, ``underflow``, ``overflow``, ``divByZero``, ``balance``, ``assert``, ``popEmptyArray``, where the default is ``all``. Multiple targets can be chosen at the same time, separated by a comma without spaces: ``underflow,overflow,assert``.
 * Command Line Interface: Only accept library addresses with a prefix of ``0x`` in ``--libraries`` commandline option.
 * Optimizer: Add rule to replace ``iszero(sub(x,y))`` by ``eq(x,y)``.
 * Parser: Report meaningful error if parsing a version pragma failed.
 * SMTChecker: Output internal and trusted external function calls in a counterexample's transaction trace.
 * SMTChecker: Show ``msg.value`` in counterexample transaction traces when greater than 0.
 * SMTChecker: Show contract name in counterexample function call.
 * SMTChecker: Support ABI functions as uninterpreted functions.
 * SMTChecker: Support try/catch statements.
 * SMTChecker: Synthesize untrusted functions called externally.
 * SMTChecker: Use checked arithmetic by default and support ``unchecked`` blocks.
 * Standard JSON: New option ``modelCheckerSettings.targets`` allows specifying which targets should be checked. The valid options are ``all``, ``constantCondition``, ``underflow``, ``overflow``, ``divByZero``, ``balance``, ``assert``, ``popEmptyArray``, where the default is ``all``. Multiple targets can be chosen at the same time, separated by a comma without spaces: ``underflow,overflow,assert``.

Bugfixes:
 * Code Generator: Fix length check when decoding malformed error data in catch clause.
 * Control Flow Graph: Fix missing error caused by read from/write to uninitialized variables.
 * SMTChecker: Fix false negatives in overriding modifiers and functions.
 * SMTChecker: Fix false negatives in the presence of inline assembly.
 * SMTChecker: Fix false negatives when analyzing external function calls.
 * SMTChecker: Fix internal error on ``block.chainid``.
 * SMTChecker: Fix internal error on pushing string literal to ``bytes`` array.
 * SMTChecker: Fix missing type constraints for block variables.
 * Type Checker: Fix infinite loop when accessing circular constants from inline assembly.
 * Type Checker: Fix internal error caused by constant structs containing mappings.
 * Type System: Disallow implicit conversion from ``uintN`` to ``intM`` when ``M > N``, and by extension, explicit conversion between the same types is also disallowed.

Build System:
 * Update the soljson.js build to emscripten 2.0.12 and boost 1.75.0.


### 0.8.0 (2020-12-16)

Breaking Changes:
 * Code Generator: All arithmetic is checked by default. These checks can be disabled using ``unchecked { ... }``.
 * Code Generator: Cause a panic if a byte array in storage is accessed whose length is encoded incorrectly.
 * Code Generator: Use ``revert`` with error signature ``Panic(uint256)`` and error codes instead of invalid opcode on failing assertions.
 * Command Line Interface: JSON fields `abi`, `devdoc`, `userdoc` and `storage-layout` are now sub-objects rather than strings.
 * Command Line Interface: Remove the ``--old-reporter`` option.
 * Command Line Interface: Remove the legacy ``--ast-json`` option. Only the ``--ast-compact-json`` option is supported now.
 * General: Enable ABI coder v2 by default.
 * General: Remove global functions ``log0``, ``log1``, ``log2``, ``log3`` and ``log4``.
 * Parser: Exponentiation is right associative. ``a**b**c`` is parsed as ``a**(b**c)``.
 * Scanner: Remove support for the ``\b``, ``\f``, and ``\v`` escape sequences.
 * Standard JSON: Remove the ``legacyAST`` option.
 * Type Checker: Function call options can only be given once.
 * Type System: Declarations with the name ``this``, ``super`` and ``_`` are disallowed, with the exception of public functions and events.
 * Type System: Disallow ``msg.data`` in ``receive()`` function.
 * Type System: Disallow ``type(super)``.
 * Type System: Disallow enums with more than 256 members.
 * Type System: Disallow explicit conversions from negative literals and literals larger than ``type(uint160).max`` to ``address`` type.
 * Type System: Disallow the ``byte`` type. It was an alias to ``bytes1``.
 * Type System: Explicit conversion to ``address`` type always returns a non-payable ``address`` type. In particular, ``address(u)``, ``address(b)``, ``address(c)`` and ``address(this)`` have the type ``address`` instead of ``address payable`` (Here ``u``, ``b``, and ``c`` are arbitrary variables of type ``uint160``, ``bytes20`` and contract type respectively.)
 * Type System: Explicit conversions between two types are disallowed if it changes more than one of sign, width or kind at the same time.
 * Type System: Explicit conversions from literals to enums are only allowed if the value fits in the enum.
 * Type System: Explicit conversions from literals to integer type is as strict as implicit conversions.
 * Type System: Introduce ``address(...).code`` to retrieve the code as ``bytes memory``. The size can be obtained via ``address(...).code.length``, but it will currently always include copying the code.
 * Type System: Introduce ``block.chainid`` for retrieving the current chain id.
 * Type System: Support ``address(...).codehash`` to retrieve the codehash of an account.
 * Type System: The global variables ``tx.origin`` and ``msg.sender`` have type ``address`` instead of ``address payable``.
 * Type System: Unary negation can only be used on signed integers, not on unsigned integers.
 * View Pure Checker: Mark ``chainid`` as view.
 * Yul: Disallow the use of reserved identifiers, such as EVM instructions, even if they are not available in the given dialect / EVM version.
 * Yul: The ``assignimmutable`` builtin in the "EVM with objects" dialect takes the base offset of the code to modify as an additional argument.

Language Features:
 * Super constructors can now be called using the member notation e.g. ``M.C(123)``.

Bugfixes:
 * Type Checker: Perform proper truncating integer arithmetic when using constants in array length expressions.

AST Changes:
 * New AST Node ``IdentifierPath`` replacing in many places the ``UserDefinedTypeName``.
 * New AST Node ``UncheckedBlock`` used for ``unchecked { ... }``.

### 0.7.6 (2020-12-16)

Language Features:
 * Code generator: Support conversion from calldata slices to memory and storage arrays.
 * Code generator: Support copying dynamically encoded structs from calldata to memory.
 * Code generator: Support copying of nested arrays from calldata to memory.
 * Scanner: Generate a parser error when comments or unicode strings contain an unbalanced or underflowing set of unicode direction override markers (LRO, RLO, LRE, RLE, PDF).
 * The fallback function can now also have a single ``calldata`` argument (equaling ``msg.data``) and return ``bytes memory`` (which will not be ABI-encoded but returned as-is).
 * Wasm backend: Add ``i32.select`` and ``i64.select`` instructions.

Compiler Features:
 * Build System: Optionally support dynamic loading of Z3 and use that mechanism for Linux release builds.
 * Code Generator: Avoid memory allocation for default value if it is not used.
 * SMTChecker: Apply constant evaluation on binary arithmetic expressions.
 * SMTChecker: Create underflow and overflow verification targets for increment/decrement in the CHC engine.
 * SMTChecker: Report struct values in counterexamples from CHC engine.
 * SMTChecker: Support early returns in the CHC engine.
 * SMTChecker: Support getters.
 * SMTChecker: Support named arguments in function calls.
 * SMTChecker: Support struct constructor.
 * Standard-Json: Move the recently introduced ``modelCheckerSettings`` key to ``settings.modelChecker``.
 * Standard-Json: Properly filter the requested output artifacts.

Bugfixes:
 * Code generator: Do not pad empty string literals with a single 32-byte zero field in the ABI coder v1.
 * NatSpec: Fix segfault when inheriting return parameter documentation for modifiers with no parameters.
 * SMTChecker: Fix cast string literals to byte arrays.
 * SMTChecker: Fix internal compiler error when doing bitwise compound assignment with string literals.
 * SMTChecker: Fix internal error when trying to generate counterexamples with old z3.
 * SMTChecker: Fix segmentation fault that could occur on certain SMT-enabled sources when no SMT solver was available.
 * SMTChecker: Fix internal error when ``bytes.push()`` is used as the LHS of an assignment.
 * Type Checker: ``super`` is not available in libraries.
 * Type Checker: Disallow leading zeroes in sized-types (e.g. ``bytes000032``), but allow them to be treated as identifiers.
 * Yul Optimizer: Fix a bug in NameSimplifier where a new name created by NameSimplifier could also be created by NameDispenser.
 * Yul Optimizer: Removed NameSimplifier from optimization steps available to users.

### 0.7.5 (2020-11-18)

Language Features:
 * Ability to select the abi coder using ``pragma abicoder v1`` and ``pragma abicoder v2``.
 * Inline Assembly: Use ``.offset`` and ``.length`` for calldata variables of dynamic array type to access their calldata offset and length (number of elements). Both of them can also be assigned to.
 * Immutable variables with literal number values are considered pure.

Compiler Features:
 * Assembler: Perform linking in assembly mode when library addresses are provided.
 * Command Line Interface: New option ``--experimental-via-ir`` allows switching compilation process to go through the Yul intermediate representation. This is highly experimental and is used for development purposes.
 * Command Line Interface: New option ``--model-checker-timeout`` sets a timeout in milliseconds for each individual query performed by the SMTChecker.
 * Command Line Interface: Report error if file could not be read in ``--standard-json`` mode.
 * Command Line interface: Report proper error for each output file which could not be written. Previously an exception was thrown, and execution aborted, on the first error.
 * SMTChecker: Add division by zero checks in the CHC engine.
 * SMTChecker: More precise analysis of external calls using ``this``.
 * SMTChecker: Support ``selector`` for expressions with value known at compile-time.
 * Standard JSON: New option ``modelCheckerSettings.timeout`` sets a timeout in milliseconds for each individual query performed by the SMTChecker.
 * Standard JSON: New option ``settings.viaIR`` allows the same switch as ``--experimental-via-ir`` on the commandline.


Bugfixes:
 * Code generator: Fix missing creation dependency tracking for abstract contracts.
 * Command Line Interface: Fix write error when the directory passed to ``--output-dir`` ends with a slash.
 * Command Line Interface: Reject duplicate libraries in ``--libraries`` option instead of arbitrarily choosing one.
 * NatSpec: Fix internal error when inheriting return parameter documentation but the parameter names differ between base and inherited.
 * SMTChecker: Fix CHC false positives when branches are used inside modifiers.
 * SMTChecker: Fix false negative in modifier applied multiple times.
 * SMTChecker: Fix incorrect counterexamples reported by the CHC engine.
 * SMTChecker: Fix internal error in the BMC engine when inherited contract from a different source unit has private state variables.
 * SMTChecker: Fix internal error on conversion from string literal to byte.
 * SMTChecker: Fix internal error when ``array.push()`` is used as the LHS of an assignment.
 * SMTChecker: Fix internal error when assigning state variable via contract's name.
 * SMTChecker: Fix internal error when using tuples of rational literals inside the conditional operator.
 * SMTChecker: Fix lack of reporting potential violations when using only the CHC engine.
 * Standard JSON: Fix library addresses specified in ``libraries`` being used for linking even if the file names do not match.

AST Changes:
 * New member ``suffix`` for inline assembly identifiers. Currently supported values are ``"slot"``, ``"offset"`` and ``"length"`` to access the components of a Solidity variable.


### 0.7.4 (2020-10-19)

Important Bugfixes:
 * Code Generator: Fix data corruption bug when copying empty byte arrays from memory or calldata to storage.


Language Features:
 * Constants can be defined at file level.


Compiler Features:
 * Command Line Interface: New option ``--model-checker-engine`` allows to choose a specific SMTChecker engine. Options are ``all`` (default), ``bmc``, ``chc`` and ``none``.
 * Control Flow Graph: Print warning for non-empty functions with unnamed return parameters that are not assigned a value in all code paths.
 * SMTChecker: Support ``keccak256``, ``sha256``, ``ripemd160`` and ``ecrecover`` in the CHC engine.
 * SMTChecker: Support inline arrays.
 * SMTChecker: Support variables ``block``, ``msg`` and ``tx`` in the CHC engine.
 * Standard JSON: New option ``modelCheckerSettings.engine`` allows to choose a specific SMTChecker engine. Options are ``all`` (default), ``bmc``, ``chc`` and ``none``.


Bugfixes:
 * Code generator: Fix ``ABIEncoderV2`` pragma from the current module affecting inherited functions and applied modifiers.
 * Code generator: Fix internal compiler error when referencing members via module name but not using the reference.
 * Code generator: Fix internal error on returning structs containing mappings from library function.
 * Code generator: Use revert instead of invalid opcode for out-of-bounds array index access in getter.
 * Contract Level Checker: Add missing check against inheriting functions with ABIEncoderV2 return types in ABIEncoderV1 contracts.
 * Name Resolver: Fix shadowing/same-name warnings for later declarations.
 * Type Checker: Allow arrays of contract types as type expressions and as arguments for ``abi.decode``.
 * Type Checker: Disallow invalid use of library names as type name.
 * Type Checker: Fix internal compiler error caused by storage parameters with nested mappings in libraries.


### 0.7.3 (2020-10-07)

Important Bugfixes:
 * Code Generator: Properly cleanup after copying dynamic-array to storage for packed types.

Compiler Features:
 * Code generator: Implemented events with function type as one of its indexed parameters.
 * General: Option to stop compilation after parsing stage. Can be used with ``solc --stop-after parsing``
 * Optimizer: Optimize ``exp`` when base is ``-1``.
 * SMTChecker: Support ``addmod`` and ``mulmod``.
 * SMTChecker: Support array slices.
 * SMTChecker: Support type conversions.


Bugfixes:
 * Fixed internal compiler errors for certain contracts involving the ``new`` expression.
 * JSON AST: Fix internal error when using ``--ast-json`` on a function with memory arguments in ABIEncoderV2 contracts.
 * Type Checker: Add missing checks for calls using types incompatible with ABIEncoderV1 in modules where ABIEncoderV2 is not enabled.
 * Type Checker: Fix internal compiler error when calling `.push(<arg>)` for a storage array with a nested mapping.


### 0.7.2 (2020-09-28)

Important Bugfixes:
 * Type Checker: Disallow two or more free functions with identical name (potentially imported and aliased) and parameter types.

Compiler Features:
 * Export compiler-generated utility sources via standard-json or combined-json.
 * Optimizer: Optimize ``exp`` when base is 0, 1 or 2.
 * SMTChecker: Keep knowledge about string literals, even through assignment, and thus support the ``.length`` property properly.
 * SMTChecker: Support ``address`` type conversion with literals, e.g. ``address(0)``.
 * SMTChecker: Support ``revert()``.
 * SMTChecker: Support ``type(T).min``, ``type(T).max``, and ``type(I).interfaceId``.
 * SMTChecker: Support compound and, or, and xor operators.
 * SMTChecker: Support events and low-level logs.
 * SMTChecker: Support fixed bytes index access.
 * SMTChecker: Support memory allocation, e.g. ``new bytes(123)``.
 * SMTChecker: Support shifts.
 * SMTChecker: Support structs.
 * Type Checker: Explain why oversized hex string literals can not be explicitly converted to a shorter ``bytesNN`` type.
 * Type Checker: More detailed error messages why implicit conversions fail.
 * Type Checker: Report position of first invalid UTF-8 sequence in ``unicode""`` literals.
 * Yul IR Generator: Report source locations related to unimplemented features.
 * Yul Optimizer: Inline into functions further down in the call graph first.
 * Yul Optimizer: Prune unused parameters in functions.
 * Yul Optimizer: Try to simplify function names.


Bugfixes:
 * Code generator: Fix internal error on stripping dynamic types from return parameters on EVM versions without ``RETURNDATACOPY``.
 * Type Checker: Add missing check against nested dynamic arrays in ABI encoding functions when ABIEncoderV2 is disabled.
 * Type Checker: Correct the error message for invalid named parameter in a call to refer to the right argument.
 * Type Checker: Disallow ``virtual`` for modifiers in libraries.
 * Name Resolver: Correct the warning for homonymous, but not shadowing declarations.
 * Type system: Fix internal error on implicit conversion of contract instance to the type of its ``super``.
 * Type system: Fix internal error on implicit conversion of string literal to a calldata string.
 * Type system: Fix named parameters in overloaded function and event calls being matched incorrectly if the order differs from the declaration.
 * ViewPureChecker: Prevent visibility check on constructors.


### 0.7.1 (2020-09-02)

Language Features:
 * Allow function definitions outside of contracts, behaving much like internal library functions.
 * Code generator: Implementing copying structs from calldata to storage.

Compiler Features:
 * SMTChecker: Add underflow and overflow as verification conditions in the CHC engine.
 * SMTChecker: Support bitwise or, xor and not operators.
 * SMTChecker: Support conditional operator.
 * Standard JSON Interface: Do not run EVM bytecode code generation, if only Yul IR or EWasm output is requested.
 * Yul Optimizer: LoopInvariantCodeMotion can move reading operations outside for-loops as long as the affected area is not modified inside the loop.
 * Yul: Report error when using non-string literals for ``datasize()``, ``dataoffset()``, ``linkersymbol()``, ``loadimmutable()``, ``setimmutable()``.

Bugfixes:
 * AST: Remove ``null`` member values also when the compiler is used in standard-json-mode.
 * General: Allow `type(Contract).name` for abstract contracts and interfaces.
 * Immutables: Disallow assigning immutables more than once during their declaration.
 * Immutables: Properly treat complex assignment and increment/decrement as both reading and writing and thus disallow it everywhere for immutable variables.
 * Optimizer: Keep side-effects of ``x`` in ``byte(a, shr(b, x))`` even if the constants ``a`` and ``b`` would make the expression zero unconditionally. This optimizer rule is very hard if not impossible to trigger in a way that it can result in invalid code, though.
 * References Resolver: Fix internal bug when using constructor for library.
 * Scanner: Fix bug where whitespace would be allowed within the ``->`` token (e.g. ``function f() -   > x {}`` becomes invalid in inline assembly and Yul).
 * SMTChecker: Fix internal error in BMC function inlining.
 * SMTChecker: Fix internal error on array implicit conversion.
 * SMTChecker: Fix internal error on fixed bytes index access.
 * SMTChecker: Fix internal error on lvalue unary operators with tuples.
 * SMTChecker: Fix internal error on tuple assignment.
 * SMTChecker: Fix internal error on tuples of one element that have tuple type.
 * SMTChecker: Fix internal error when using imported code.
 * SMTChecker: Fix soundness of array ``pop``.
 * Type Checker: Disallow ``using for`` directive inside interfaces.
 * Type Checker: Disallow signed literals as exponent in exponentiation operator.
 * Type Checker: Disallow structs containing nested mapping in memory as parameters for library functions.
 * Yul Optimizer: Ensure that Yul keywords are not mistakenly used by the NameDispenser and VarNameCleaners. The bug would manifest as uncompilable code.
 * Yul Optimizer: Make function inlining order more resilient to whether or not unrelated source files are present.


### 0.7.0 (2020-07-28)

Breaking changes:
 * Inline Assembly: Disallow ``.`` in user-defined function and variable names.
 * Inline Assembly: Slot and offset of storage pointer variable ``x`` are accessed via ``x.slot`` and ``x.offset`` instead of ``x_slot`` and ``x_offset``.
 * JSON AST: Mark hex string literals with ``kind: "hexString"``.
 * JSON AST: Remove members with ``null`` value from JSON output.
 * Parser: Disallow ``gwei`` as identifier.
 * Parser: Disallow dot syntax for ``value`` and ``gas``.
 * Parser: Disallow non-printable characters in string literals.
 * Parser: Introduce Unicode string literals: ``unicode"😃"``.
 * Parser: NatSpec comments on variables are only allowed for public state variables.
 * Parser: Remove the ``finney`` and ``szabo`` denominations.
 * Parser: Remove the identifier ``now`` (replaced by ``block.timestamp``).
 * Reference Resolver: ``using A for B`` only affects the contract it is mentioned in and not all derived contracts
 * Type Checker: Disallow ``virtual`` for library functions.
 * Type Checker: Disallow assignments to state variables that contain nested mappings.
 * Type checker: Disallow events with same name and parameter types in inheritance hierarchy.
 * Type Checker: Disallow shifts by signed types.
 * Type Checker: Disallow structs and arrays in memory or calldata if they contain nested mappings.
 * Type Checker: Exponentiation and shifts of literals by non-literals will always use ``uint256`` or ``int256`` as a type.
 * Yul: Disallow consecutive and trailing dots in identifiers. Leading dots were already disallowed.
 * Yul: Disallow EVM instruction `pc()`.


Language Features:
 * Inheritance: Allow overrides to have stricter state mutability: ``view`` can override ``nonpayable`` and ``pure`` can override ``view``.
 * Parser: Deprecate visibility for constructors.
 * State mutability: Do not issue recommendation for stricter mutability for virtual functions but do issue it for functions that override.


Compiler Features:
 * SMTChecker: Report multi-transaction counterexamples including the function calls that initiate the transactions. This does not include concrete values for reference types and reentrant calls.
 * Variable declarations using the ``var`` keyword are not recognized anymore.


Bugfixes:
 * Immutables: Fix internal compiler error when immutables are not assigned.
 * Inheritance: Disallow public state variables overwriting ``pure`` functions.
 * NatSpec: Constructors and functions have consistent userdoc output.
 * SMTChecker: Fix internal error when assigning to a 1-tuple.
 * SMTChecker: Fix internal error when tuples have extra effectless parenthesis.
 * State Mutability: Constant public state variables are considered ``pure`` functions.
 * Type Checker: Fixing deduction issues on function types when function call has named arguments.


### 0.6.12 (2020-07-22)

Language Features:
 * NatSpec: Implement tag ``@inheritdoc`` to copy documentation from a specific base contract.
 * Wasm backend: Add ``i32.ctz``, ``i64.ctz``, ``i32.popcnt``, and ``i64.popcnt``.


Compiler Features:
 * Code Generator: Avoid double cleanup when copying to memory.
 * Code Generator: Evaluate ``keccak256`` of string literals at compile-time.
 * Optimizer: Add rule to remove shifts inside the byte opcode.
 * Peephole Optimizer: Add rule to remove swap after dup.
 * Peephole Optimizer: Remove unnecessary masking of tags.
 * Yul EVM Code Transform: Free stack slots directly after visiting the right-hand-side of variable declarations instead of at the end of the statement only.


Bugfixes:
 * SMTChecker: Fix error in events with indices of type static array.
 * SMTChecker: Fix internal error in sequential storage array pushes (``push().push()``).
 * SMTChecker: Fix internal error when using bitwise operators on fixed bytes type.
 * SMTChecker: Fix internal error when using compound bitwise operator assignments on array indices inside branches.
 * Type Checker: Fix internal compiler error related to oversized types.
 * Type Checker: Fix overload resolution in combination with ``{value: ...}``.


Build System:
 * Update internal dependency of jsoncpp to 1.9.3.


### 0.6.11 (2020-07-07)


Language Features:
 * General: Add unit denomination ``gwei``
 * Yul: Support ``linkersymbol`` builtin in standalone assembly mode to refer to library addresses.
 * Yul: Support using string literals exceeding 32 bytes as literal arguments for builtins.


Compiler Features:
 * NatSpec: Add fields ``kind`` and ``version`` to the JSON output.
 * NatSpec: Inherit tags from unique base functions if derived function does not provide any.
 * Commandline Interface: Prevent some incompatible commandline options from being used together.
 * NatSpec: Support NatSpec comments on events.
 * Yul Optimizer: Store knowledge about storage / memory after ``a := sload(x)`` / ``a := mload(x)``.
 * SMTChecker: Support external calls to unknown code.
 * Source Maps: Also tag jumps into and out of Yul functions as jumps into and out of functions.


Bugfixes:
 * NatSpec: Do not consider ``////`` and ``/***`` as NatSpec comments.
 * Type Checker: Disallow constructor parameters with ``calldata`` data location.
 * Type Checker: Do not disallow assigning to calldata variables.
 * Type Checker: Fix internal error related to ``using for`` applied to non-libraries.
 * Wasm backend: Fix code generation for for-loops with pre statements.
 * Wasm backend: Properly support both ``i32.drop`` and ``i64.drop``, and remove ``drop``.
 * Yul: Disallow the same variable to occur multiple times on the left-hand side of an assignment.
 * Yul: Fix source location of variable multi-assignment.


### 0.6.10 (2020-06-11)

Important Bugfixes:
 * Fixed a bug related to internal library functions with ``calldata`` parameters called via ``using for``.


Compiler Features:
 * Commandline Interface: Re-group help screen.
 * Output compilation error codes in standard-json and when using ``--error-codes``.
 * Yul: Raise warning for switch statements that only have a default and no other cases.


Bugfixes:
 * SMTChecker: Fix internal error when encoding tuples of tuples.
 * SMTChecker: Fix aliasing soundness after pushing to an array pointer.
 * Type system: Fix internal compiler error on calling externally a function that returns variables with calldata location.
 * Type system: Fix bug where a bound function was not found if ``using for`` is applied to explicit reference types.


### 0.6.9 (2020-06-04)

Language Features:
 * Permit calldata location for all variables.
 * NatSpec: Support NatSpec comments on state variables.
 * Yul: EVM instruction `pc()` is marked deprecated and will be removed in the next breaking release.


Compiler Features:
 * Build system: Update the soljson.js build to emscripten 1.39.15 and boost 1.73.0 and include Z3 for integrated SMTChecker support without the callback mechanism.
 * Build system: Switch the emscripten build from the fastcomp backend to the upstream backend.
 * Code Generator: Do not introduce new internal source references for small compiler routines.
 * Commandline Interface: Adds new option ``--base-path PATH`` to use the given path as the root of the source tree (defaults to the root of the filesystem).
 * SMTChecker: Support array ``length``.
 * SMTChecker: Support array ``push`` and ``pop``.
 * SMTChecker: General support to BitVectors and the bitwise ``and`` operator.


Bugfixes:
 * Code Generator: Trigger proper unimplemented errors on certain array copy operations.
 * Commandline Interface: Fix internal error when using ``--assemble`` or ``--yul`` options with ``--machine ewasm`` but without specifying ``--yul-dialect``.
 * NatSpec: DocString block is terminated when encountering an empty line.
 * Optimizer: Fixed a bug in BlockDeDuplicator.
 * Scanner: Fix bug when two empty NatSpec comments lead to scanning past EOL.
 * SMTChecker: Fix internal error on try/catch clauses with parameters.
 * SMTChecker: Fix internal error when applying arithmetic operators to fixed point variables.
 * SMTChecker: Fix internal error when assigning to index access inside branches.
 * SMTChecker: Fix internal error when short circuiting Boolean expressions with function calls in state variable initialization.
 * Type Checker: Disallow assignments to storage variables of type ``mapping``.
 * Type Checker: Disallow inline arrays of non-nameable types.
 * Type Checker: Disallow usage of override with non-public state variables.
 * Type Checker: Fix internal compiler error when accessing members of array slices.
 * Type Checker: Fix internal compiler error when forward referencing non-literal constants from inline assembly.
 * Type Checker: Fix internal compiler error when trying to decode too large static arrays.
 * Type Checker: Fix wrong compiler error when referencing an overridden function without calling it.


### 0.6.8 (2020-05-14)

Important Bugfixes:
 * Add missing callvalue check to the creation code of a contract that does not define a constructor but has a base that does define a constructor.
 * Disallow array slices of arrays with dynamically encoded base types.
 * String literals containing backslash characters can no longer cause incorrect code to be generated when passed directly to function calls or encoding functions when ABIEncoderV2 is active.


Language Features:
 * Implemented ``type(T).min`` and ``type(T).max`` for every integer type ``T`` that returns the smallest and largest value representable by the type.


Compiler Features:
 * Commandline Interface: Don't ignore `--yul-optimizations` in assembly mode.
 * Allow using abi encoding functions for calldata array slices without explicit casts.
 * Wasm binary output: Implement ``br`` and ``br_if``.


Bugfixes:
 * ABI: Skip ``private`` or ``internal`` constructors.
 * Fixed an "Assembly Exception in Bytecode" error where requested functions were generated twice.
 * Natspec: Fixed a bug that ignored ``@return`` tag when no other developer-documentation tags were present.
 * Type Checker: Checks if a literal exponent in the ``**`` operation is too large or fractional.
 * Type Checker: Disallow accessing ``runtimeCode`` for contract types that contain immutable state variables.
 * Yul Assembler: Fix source location of variable declarations without value.


### 0.6.7 (2020-05-04)

Language Features:
 * Add support for EIP 165 interface identifiers with `type(I).interfaceId`.
 * Allow virtual modifiers inside abstract contracts to have empty body.


Compiler Features:
 * Optimizer: Simplify repeated AND and OR operations.
 * Standard Json Input: Support the prefix ``file://`` in the field ``urls``.
 * Add option to specify optimization steps to be performed by Yul optimizer with `--yul-optimizations` in the commandline interface or `optimizer.details.yulDetails.optimizerSteps` in standard-json.

Bugfixes:
 * SMTChecker: Fix internal error when fixed points are used.
 * SMTChecker: Fix internal error when using array slices.
 * Type Checker: Disallow ``virtual`` and ``override`` for constructors.
 * Type Checker: Fix several internal errors by performing size and recursiveness checks of types before the full type checking.
 * Type Checker: Fix internal error when assigning to empty tuples.
 * Type Checker: Fix internal error when applying unary operators to tuples with empty components.
 * Type Checker: Perform recursiveness check on structs declared at the file level.

Build System:
 * soltest.sh: ``SOLIDITY_BUILD_DIR`` is no longer relative to ``REPO_ROOT`` to allow for build directories outside of the source tree.



### 0.6.6 (2020-04-09)

Important Bugfixes:
 * Fix tuple assignments with components occupying multiple stack slots and different stack size on left- and right-hand-side.


Bugfixes:
 * AST export: Export `immutable` property in the field `mutability`.
 * SMTChecker: Fix internal error in the CHC engine when calling inherited functions internally.
 * Type Checker: Error when trying to encode functions with call options gas and value set.



### 0.6.5 (2020-04-06)

Important Bugfixes:
 * Code Generator: Restrict the length of dynamic memory arrays to 64 bits during creation at runtime fixing a possible overflow.


Language Features:
 * Allow local storage variables to be declared without initialization, as long as they are assigned before they are accessed.
 * State variables can be marked ``immutable`` which causes them to be read-only, but assignable in the constructor. The value will be stored directly in the code.


Compiler Features:
 * Commandline Interface: Enable output of storage layout with `--storage-layout`.
 * Metadata: Added support for IPFS hashes of large files that need to be split in multiple chunks.


Bugfixes:
 * Inheritance: Allow public state variables to override functions with dynamic memory types in their return values.
 * Inline Assembly: Fix internal error when accessing invalid constant variables.
 * Inline Assembly: Fix internal error when accessing functions.
 * JSON AST: Always add pointer suffix for memory reference types.
 * Reference Resolver: Fix internal error when accessing invalid struct members.
 * Type Checker: Fix internal errors when assigning nested tuples.


### 0.6.4 (2020-03-10)

Language Features:
 * General: Deprecated `value(...)` and `gas(...)` in favor of `{value: ...}` and `{gas: ...}`
 * Inline Assembly: Allow assigning to `_slot` of local storage variable pointers.
 * Inline Assembly: Perform control flow analysis on inline assembly. Allows storage returns to be set in assembly only.


Compiler Features:
 * AssemblyStack: Support for source locations (source mappings) and thus debugging Yul sources.
 * Commandline Interface: Enable output of experimental optimized IR via ``--ir-optimized``.


Bugfixes:
 * Inheritance: Fix incorrect error on calling unimplemented base functions.
 * Reference Resolver: Fix scoping issue following try/catch statements.
 * Standard-JSON-Interface: Fix a bug related to empty filenames and imports.
 * SMTChecker: Fix internal errors when analysing tuples.
 * Yul AST Import: correctly import blocks as statements, switch statements and string literals.

### 0.6.3 (2020-02-18)

Language Features:
 * Allow contract types and enums as keys for mappings.
 * Allow function selectors to be used as compile-time constants.
 * Report source locations for structured documentation errors.


Compiler Features:
 * AST: Add a new node for doxygen-style, structured documentation that can be received by contract, function, event and modifier definitions.
 * Code Generator: Use ``calldatacopy`` instead of ``codecopy`` to zero out memory past input.
 * Debug: Provide reason strings for compiler-generated internal reverts when using the ``--revert-strings`` option or the ``settings.debug.revertStrings`` setting on ``debug`` mode.
 * Yul Optimizer: Prune functions that call each other but are otherwise unreferenced.
 * SMTChecker: CHC support to internal function calls.


Bugfixes:
 * Assembly: Added missing `source` field to legacy assembly json output to complete the source reference.
 * Parser: Fix an internal error for ``abstract`` without ``contract``.
 * Type Checker: Make invalid calls to uncallable types fatal errors instead of regular.


### 0.6.2 (2020-01-27)

Language Features:
 * Allow accessing external functions via contract and interface names to obtain their selector.
 * Allow interfaces to inherit from other interfaces
 * Allow gas and value to be set in external function calls using ``c.f{gas: 10000, value: 4 ether}()``.
 * Allow specifying the ``salt`` for contract creations and thus the ``create2`` opcode using ``new C{salt: 0x1234, value: 1 ether}(arg1, arg2)``.
 * Inline Assembly: Support literals ``true`` and ``false``.


Compiler Features:
 * LLL: The LLL compiler has been removed.
 * General: Raise warning if runtime bytecode exceeds 24576 bytes (a limit introduced in Spurious Dragon).
 * General: Support compiling starting from an imported AST. Among others, this can be used for mutation testing.
 * Yul Optimizer: Apply penalty when trying to rematerialize into loops.


Bugfixes:
 * Commandline interface: Only activate yul optimizer if ``--optimize`` is given.
 * Fixes internal compiler error on explicitly calling unimplemented base functions.


Build System:
 * Switch to building soljson.js with an embedded base64-encoded wasm binary.


### 0.6.1 (2020-01-02)

Bugfixes:
 * Yul Optimizer: Fix bug in redundant assignment remover in combination with break and continue statements.


### 0.6.0 (2019-12-17)

Breaking changes:
 * ABI: Remove the deprecated ``constant`` and ``payable`` fields.
 * ABI: The ``type`` field is now required and no longer specified to default to ``function``.
 * AST: Inline assembly is exported as structured JSON instead of plain string.
 * C API (``libsolc``): Introduce context parameter to both ``solidity_compile`` and the callback.
 * C API (``libsolc``): The provided callback now takes two parameters, kind and data. The callback can then be used for multiple purposes, such has file imports and SMT queries.
 * C API (``libsolc``): ``solidity_free`` was renamed to ``solidity_reset``. Functions ``solidity_alloc`` and ``solidity_free`` were added.
 * C API (``libsolc``): ``solidity_compile`` now returns a string that must be explicitly freed via ``solidity_free()``
 * Commandline Interface: Remove the text-based AST printer (``--ast``).
 * Commandline Interface: Switch to the new error reporter by default. ``--old-reporter`` falls back to the deprecated old error reporter.
 * Commandline Interface: Add option to disable or choose hash method between IPFS and Swarm for the bytecode metadata.
 * General: Disallow explicit conversions from external function types to ``address`` and add a member called ``address`` to them as replacement.
 * General: Enable Yul optimizer as part of standard optimization.
 * General: New reserved keywords: ``override``, ``receive``, and ``virtual``.
 * General: ``private`` cannot be used together with ``virtual``.
 * General: Split unnamed fallback functions into two cases defined using ``fallback()`` and ``receive()``.
 * Inheritance: State variable shadowing is now disallowed.
 * Inline Assembly: Only strict inline assembly is allowed.
 * Inline Assembly: Variable declarations cannot shadow declarations outside the assembly block.
 * JSON AST: Replace ``superFunction`` attribute by ``baseFunctions``.
 * Natspec JSON Interface: Properly support multiple ``@return`` statements in ``@dev`` documentation and enforce named return parameters to be mentioned documentation.
 * Source mappings: Add "modifier depth" as a fifth field in the source mappings.
 * Standard JSON Interface: Add option to disable or choose hash method between IPFS and Swarm for the bytecode metadata.
 * Syntax: ``push(element)`` for dynamic storage arrays do not return the new length anymore.
 * Syntax: Abstract contracts need to be marked explicitly as abstract by using the ``abstract`` keyword.
 * Syntax: ``length`` member of arrays is now always read-only, even for storage arrays.
 * Type Checker: Resulting type of exponentiation is equal to the type of the base. Also allow signed types for the base.

Language Features:
 * Allow explicit conversions from ``address`` to ``address payable`` via ``payable(...)``.
 * Allow global enums and structs.
 * Allow public variables to override external functions.
 * Allow underscores as delimiters in hex strings.
 * Allow to react on failing external calls using ``try`` and ``catch``.
 * Introduce syntax for array slices and implement them for dynamic calldata arrays.
 * Introduce ``push()`` for dynamic storage arrays. It returns a reference to the newly allocated element, if applicable.
 * Introduce ``virtual`` and ``override`` keywords.
 * Modify ``push(element)`` for dynamic storage arrays such that it does not return the new length anymore.
 * Yul: Introduce ``leave`` statement that exits the current function.
 * JSON AST: Add the function selector of each externally-visible FunctonDefinition to the AST JSON export.

Compiler Features:
 * Allow revert strings to be stripped from the binary using the ``--revert-strings`` option or the ``settings.debug.revertStrings`` setting.
 * ABIEncoderV2: Do not warn about enabled ABIEncoderV2 anymore (the pragma is still needed, though).


### 0.5.17 (2020-03-17)

Bugfixes:
 * Type Checker: Disallow overriding of private functions.


### 0.5.16 (2020-01-02)

Backported Bugfixes:
 * Yul Optimizer: Fix bug in redundant assignment remover in combination with break and continue statements.


### 0.5.15 (2019-12-17)

Bugfixes:
 * Yul Optimizer: Fix incorrect redundant load optimization crossing user-defined functions that contain for-loops with memory / storage writes.

### 0.5.14 (2019-12-09)

Language Features:
 * Allow to obtain the selector of public or external library functions via a member ``.selector``.
 * Inline Assembly: Support constants that reference other constants.
 * Parser: Allow splitting hexadecimal and regular string literals into multiple parts.


Compiler Features:
 * Commandline Interface: Allow translation from yul / strict assembly to EWasm using ``solc --yul --yul-dialect evm --machine ewasm``
 * Set the default EVM version to "Istanbul".
 * SMTChecker: Add support to constructors including constructor inheritance.
 * Yul: When compiling via Yul, string literals from the Solidity code are kept as string literals if every character is safely printable.
 * Yul Optimizer: Perform loop-invariant code motion.


Bugfixes:
 * SMTChecker: Fix internal error when using ``abi.decode``.
 * SMTChecker: Fix internal error when using arrays or mappings of functions.
 * SMTChecker: Fix internal error in array of structs type.
 * Version Checker: ``^0`` should match ``0.5.0``, but no prerelease.
 * Yul: Consider infinite loops and recursion to be not removable.


Build System:
 * Update to emscripten version 1.39.3.


### 0.5.13 (2019-11-14)

Language Features:
 * Allow to obtain the address of a linked library with ``address(LibraryName)``.


Compiler Features:
 * Code Generator: Use SELFBALANCE opcode for ``address(this).balance`` if using Istanbul EVM.
 * EWasm: Experimental EWasm binary output via ``--ewasm`` and as documented in standard-json.
 * SMTChecker: Add break/continue support to the CHC engine.
 * SMTChecker: Support assignments to multi-dimensional arrays and mappings.
 * SMTChecker: Support inheritance and function overriding.
 * Standard JSON Interface: Output the storage layout of a contract when artifact ``storageLayout`` is requested.
 * TypeChecker: List possible candidates when overload resolution fails.
 * TypeChecker: Disallow variables of library types.

Bugfixes:
 * Code Generator: Fixed a faulty assert that would wrongly trigger for array sizes exceeding unsigned integer.
 * SMTChecker: Fix internal error when accessing indices of fixed bytes.
 * SMTChecker: Fix internal error when using function pointers as arguments.
 * SMTChecker: Fix internal error when implicitly converting string literals to fixed bytes.
 * Type Checker: Disallow constructor of the same class to be used as modifier.
 * Type Checker: Treat magic variables as unknown identifiers in inline assembly.
 * Code Generator: Fix internal error when trying to convert ``super`` to a different type


### 0.5.12 (2019-10-01)

Language Features:
 * Type Checker: Allow assignment to external function arguments except for reference types.


Compiler Features:
 * ABI Output: Change sorting order of functions from selector to kind, name.
 * Optimizer: Add rule that replaces the BYTE opcode by 0 if the first argument is larger than 31.
 * SMTChecker: Add loop support to the CHC engine.
 * Yul Optimizer: Take side-effect-freeness of user-defined functions into account.
 * Yul Optimizer: Remove redundant mload/sload operations.
 * Yul Optimizer: Use the fact that branch conditions have certain value inside the branch.


Bugfixes:
 * Code Generator: Fix internal error when popping a dynamic storage array of mappings.
 * Name Resolver: Fix wrong source location when warning on shadowed aliases in import declarations.
 * Scanner: Fix multi-line natspec comment parsing with triple slashes when file is encoded with CRLF instead of LF.
 * Type System: Fix arrays of recursive structs.
 * Yul Optimizer: Fix reordering bug in connection with shifted one and mul/div-instructions in for loop conditions.


### 0.5.11 (2019-08-12)


Language Features:
 * Inline Assembly: Support direct constants of value type in inline assembly.

Compiler Features:
 * ABI: Additional internal type info in the field ``internalType``.
 * eWasm: Highly experimental eWasm output using ``--ewasm`` in the commandline interface or output selection of ``ewasm.wast`` in standard-json.
 * Metadata: Update the swarm hash to the current specification, changes ``bzzr0`` to ``bzzr1`` and urls to use ``bzz-raw://``.
 * Standard JSON Interface: Compile only selected sources and contracts.
 * Standard JSON Interface: Provide secondary error locations (e.g. the source position of other conflicting declarations).
 * SMTChecker: Do not erase knowledge about storage pointers if another storage pointer is assigned.
 * SMTChecker: Support string literal type.
 * SMTChecker: New Horn-based algorithm that proves assertions via multi-transaction contract invariants.
 * Standard JSON Interface: Provide AST even on errors if ``--error-recovery`` commandline switch or StandardCompiler `settings.parserErrorRecovery` is true.
 * Yul Optimizer: Do not inline function if it would result in expressions being duplicated that are not cheap.


Bugfixes:
 * ABI decoder: Ensure that decoded arrays always point to distinct memory locations.
 * Code Generator: Treat dynamically encoded but statically sized arrays and structs in calldata properly.
 * SMTChecker: Fix internal error when inlining functions that contain tuple expressions.
 * SMTChecker: Fix pointer knowledge erasing in loops.
 * SMTChecker: Fix internal error when using compound bitwise assignment operators inside branches.
 * SMTChecker: Fix internal error when inlining a function that returns a tuple containing an unsupported type inside a branch.
 * SMTChecker: Fix internal error when inlining functions that use state variables and belong to a different source.
 * SMTChecker: Fix internal error when reporting counterexamples concerning state variables from different source files.
 * SMTChecker: Fix SMT sort mismatch when using string literals.
 * View/Pure Checker: Properly detect state variable access through base class.
 * Yul Analyzer: Check availability of data objects already in analysis phase.
 * Yul Optimizer: Fix an issue where memory-accessing code was removed even though ``msize`` was used in the program.


### 0.5.10 (2019-06-25)

Important Bugfixes:
 * ABIEncoderV2: Fix incorrect abi encoding of storage array of data type that occupy multiple storage slots
 * Code Generator: Properly zero out higher order bits in elements of an array of negative numbers when assigning to storage and converting the type at the same time.


Compiler Features:
 * Commandline Interface: Experimental parser error recovery via the ``--error-recovery`` commandline switch or StandardCompiler `settings.parserErrorRecovery` boolean.
 * Optimizer: Add rule to simplify ``SUB(~0, X)`` to ``NOT(X)``.
 * Yul Optimizer: Make the optimizer work for all dialects of Yul including eWasm.


Bugfixes:
 * Type Checker: Set state mutability of the function type members ``gas`` and ``value`` to pure (while their return type inherits state mutability from the function type).
 * Yul / Inline Assembly Parser: Disallow trailing commas in function call arguments.


Build System:
 * Attempt to use stock Z3 cmake files to find Z3 and only fall back to manual discovery.
 * CMake: use imported targets for boost.
 * Emscripten build: upgrade to boost 1.70.
 * Generate a cmake error for gcc versions older than 5.0.



### 0.5.9 (2019-05-28)

Language Features:
 * Inline Assembly: Revert change introduced in 0.5.7: The ``callvalue()`` instruction does not require ``payable`` anymore.
 * Static Analyzer: Disallow libraries calling themselves externally.


Compiler Features:
 * Assembler: Encode the compiler version in the deployed bytecode.
 * Code Generator: Fix handling of structs of dynamic size as constructor parameters.
 * Inline Assembly: Disallow the combination of ``msize()`` and the Yul optimizer.
 * Metadata: Add IPFS hashes of source files.
 * Optimizer: Add rule to simplify SHL/SHR combinations.
 * Optimizer: Add rules for multiplication and division by left-shifted one.
 * SMTChecker: Support inherited state variables.
 * SMTChecker: Support tuples and function calls with multiple return values.
 * SMTChecker: Support ``delete``.
 * SMTChecker: Inline external function calls to ``this``.
 * Yul Optimizer: Simplify single-run ``for`` loops to ``if`` statements.
 * Yul Optimizer: Optimize representation of numbers.
 * Yul Optimizer: Do not inline recursive functions.
 * Yul Optimizer: Do not remove instructions that affect ``msize()`` if ``msize()`` is used.

Bugfixes:
 * Code Generator: Explicitly turn uninitialized internal function pointers into invalid functions when loaded from storage.
 * Code Generator: Fix assertion failure when assigning structs containing array of mapping.
 * Compiler Internals: Reset the Yul string repository before each compilation, freeing up memory.
 * SMTChecker: Fix bad cast in base constructor modifier.
 * SMTChecker: Fix internal error when visiting state variable inherited from base class.
 * SMTChecker: Fix internal error in fixed point operations.
 * SMTChecker: Fix internal error in assignment to unsupported type.
 * SMTChecker: Fix internal error in branching when inlining function calls that modify local variables.


### 0.5.8 (2019-04-30)

Important Bugfixes:
 * Code Generator: Fix initialization routine of uninitialized internal function pointers in constructor context.
 * Yul Optimizer: Fix SSA transform for multi-assignments.


Language Features:
 * ABIEncoderV2: Implement encoding of calldata arrays and structs.
 * Code Generation: Implement copying recursive structs from storage to memory.
 * Yul: Disallow function definitions inside for-loop init blocks.


Compiler Features:
 * ABI Decoder: Raise a runtime error on dirty inputs when using the experimental decoder.
 * Optimizer: Add rule for shifts by constants larger than 255 for Constantinople.
 * Optimizer: Add rule to simplify certain ANDs and SHL combinations
 * SMTChecker: Support arithmetic compound assignment operators.
 * SMTChecker: Support unary increment and decrement for array and mapping access.
 * SMTChecker: Show unsupported warning for inline assembly blocks.
 * SMTChecker: Support mod.
 * SMTChecker: Support ``contract`` type.
 * SMTChecker: Support ``this`` as address.
 * SMTChecker: Support address members.
 * Standard JSON Interface: Metadata settings now re-produce the original ``"useLiteralContent"`` setting from the compilation input.
 * Yul: Adds break and continue keywords to for-loop syntax.
 * Yul: Support ``.`` as part of identifiers.
 * Yul Optimizer: Adds steps for detecting and removing of dead code.
 * Yul Code Generator: Directly jump over a series of function definitions (instead of jumping over each one)


Bugfixes:
 * SMTChecker: Implement Boolean short-circuiting.
 * SMTChecker: SSA control-flow did not take into account state variables that were modified inside inlined functions that were called inside branches.
 * Type System: Use correct type name for contracts in event parameters when used in libraries. This affected code generation.
 * Type System: Allow direct call to base class functions that have overloads.
 * Type System: Warn about shadowing builtin variables if user variables are named ``this`` or ``super``.
 * Yul: Properly register functions and disallow shadowing between function variables and variables in the outside scope.


Build System:
 * Soltest: Add commandline option `--test` / `-t` to isoltest which takes a string that allows filtering unit tests.
 * soltest.sh: allow environment variable ``SOLIDITY_BUILD_DIR`` to specify build folder and add ``--help`` usage.


### 0.5.7 (2019-03-26)

Important Bugfixes:
 * ABIEncoderV2: Fix bugs related to loading short value types from storage when encoding an array or struct from storage.
 * ABIEncoderV2: Fix buffer overflow problem when encoding packed array from storage.
 * Optimizer: Fix wrong ordering of arguments in byte optimization rule for constants.


Language Features:
 * Function calls with named arguments now work with overloaded functions.


Compiler Features:
 * Inline Assembly: Issue error when using ``callvalue()`` inside nonpayable function (in the same way that ``msg.value`` already does).
 * Standard JSON Interface: Support "Yul" as input language.
 * SMTChecker: Show callstack together with model if applicable.
 * SMTChecker: Support modifiers.
 * Yul Optimizer: Enable stack allocation optimization by default if Yul optimizer is active (disable in ``yulDetails``).


Bugfixes:
 * Code Generator: Defensively pad memory for ``type(Contract).name`` to multiples of 32.
 * Type System: Detect and disallow internal function pointers as parameters for public/external library functions, even when they are nested/wrapped in structs, arrays or other types.
 * Yul Optimizer: Properly determine whether a variable can be eliminated during stack compression pass.
 * Yul / Inline Assembly Parser: Disallow more than one case statement with the same label inside a switch based on the label's integer value.


Build System:
 * Install scripts: Fix boost repository URL for CentOS 6.
 * Soltest: Fix hex string update in soltest.


### 0.5.6 (2019-03-13)

Important Bugfixes:
 * Yul Optimizer: Fix visitation order bug for the structural simplifier.
 * Optimizer: Fix overflow in optimization rule that simplifies double shift by constant.

Language Features:
 * Allow calldata arrays with dynamically encoded base types with ABIEncoderV2.
 * Allow dynamically encoded calldata structs with ABIEncoderV2.


Compiler Features:
 * Optimizer: Add rules for ``lt``-comparisons with constants.
 * Peephole Optimizer: Remove double ``iszero`` before ``jumpi``.
 * SMTChecker: Support enums without typecast.
 * SMTChecker: Support one-dimensional arrays.
 * Type Checker: Provide better error messages for some literal conversions.
 * Yul Optimizer: Add rule to remove empty default switch cases.
 * Yul Optimizer: Add rule to remove empty cases if no default exists.
 * Yul Optimizer: Add rule to replace a switch with no cases with ``pop(expression)``.


Bugfixes:
 * JSON ABI: Json description of library ABIs no longer contains functions with internal types like storage structs.
 * SMTChecker: Fix internal compiler error when contract contains too large rational number.
 * Type system: Detect if a contract's base uses types that require the experimental abi encoder while the contract still uses the old encoder.


Build System:
 * Soltest: Add support for arrays in function signatures.
 * Soltest: Add support for struct arrays in function signatures.
 * Soltest: Add support for left-aligned, unpadded hex string literals.

### 0.5.5 (2019-03-05)

Language Features:
 * Add support for getters of mappings with ``string`` or ``bytes`` key types.
 * Meta programming: Provide access to the name of contracts via ``type(C).name``.


Compiler Features:
 * Support ``petersburg`` as ``evmVersion`` and set as default.
 * Commandline Interface: Option to activate the experimental yul optimizer using ``-optimize-yul``.
 * Inline Assembly: Consider ``extcodehash`` as part of Constantinople.
 * Inline Assembly: Instructions unavailable to the currently configured EVM are errors now.
 * SMTChecker: Do not report underflow/overflow if they always revert. This removes false positives when using ``SafeMath``.
 * Standard JSON Interface: Allow retrieving metadata without triggering bytecode generation.
 * Standard JSON Interface: Provide fine-grained control over the optimizer via the settings.
 * Static Analyzer: Warn about expressions with custom types when they have no effect.
 * Optimizer: Add new rules with constants including ``LT``, ``GT``, ``AND`` and ``BYTE``.
 * Optimizer: Add rule for shifts with constants for Constantinople.
 * Optimizer: Combine multiple shifts with constant shift-by values into one.
 * Optimizer: Do not mask with 160-bits after ``CREATE`` and ``CREATE2`` as they are guaranteed to return an address or 0.
 * Optimizer: Support shifts in the constant optimiser for Constantinople.
 * Yul Optimizer: Add rule to replace switch statements with literals by matching case body.


Bugfixes:
 * ABIEncoderV2: Fix internal error related to bare delegatecall.
 * ABIEncoderV2: Fix internal error related to ecrecover.
 * ABIEncoderV2: Fix internal error related to mappings as library parameters.
 * ABIEncoderV2: Fix invalid signature for events containing structs emitted in libraries.
 * Inline Assembly: Proper error message for missing variables.
 * Optimizer: Fix internal error related to unused tag removal across assemblies. This never generated any invalid code.
 * SMTChecker: Fix crash related to statically-sized arrays.
 * TypeChecker: Fix internal error and disallow index access on contracts and libraries.
 * Yul: Properly detect name clashes with functions before their declaration.
 * Yul: Take built-in functions into account in the compilability checker.
 * Yul Optimizer: Properly take reassignments to variables in sub-expressions into account when replacing in the ExpressionSimplifier.


Build System:
 * Soltest: Add support for left-aligned, padded hex literals.
 * Soltest: Add support for right-aligned, padded boolean literals.

### 0.5.4 (2019-02-12)

Language Features:
 * Allow calldata structs without dynamically encoded members with ABIEncoderV2.


Compiler Features:
 * ABIEncoderV2: Implement packed encoding.
 * C API (``libsolc`` / raw ``soljson.js``): Introduce ``solidity_free`` method which releases all internal buffers to save memory.
 * Commandline Interface: Adds new option ``--new-reporter`` for improved diagnostics formatting
   along with ``--color`` and ``--no-color`` for colorized output to be forced (or explicitly disabled).


Bugfixes:
 * Code Generator: Defensively pad allocation of creationCode and runtimeCode to multiples of 32 bytes.
 * Commandline Interface: Allow yul optimizer only for strict assembly.
 * Parser: Disallow empty import statements.
 * Type Checker: Disallow mappings with data locations other than ``storage``.
 * Type Checker: Fix internal error when a struct array index does not fit into a uint256.
 * Type System: Properly report packed encoded size for arrays and structs (mostly unused until now).


Build System:
 * Add support for continuous fuzzing via Google oss-fuzz
 * SMT: If using Z3, require version 4.6.0 or newer.
 * Soltest: Add parser that is used in the file-based unit test environment.
 * Ubuntu PPA Packages: Use CVC4 as SMT solver instead of Z3


### 0.5.3 (2019-01-22)

Language Features:
 * Provide access to creation and runtime code of contracts via ``type(C).creationCode`` / ``type(C).runtimeCode``.


Compiler Features:
 * Control Flow Graph: Warn about unreachable code.
 * SMTChecker: Support basic typecasts without truncation.
 * SMTChecker: Support external function calls and erase all knowledge regarding storage variables and references.


Bugfixes:
 * Emscripten: Split simplification rule initialization up further to work around issues with soljson.js in some browsers.
 * Type Checker: Disallow calldata structs until implemented.
 * Type Checker: Return type error if fixed point encoding is attempted instead of throwing ``UnimplementedFeatureError``.
 * Yul: Check that arguments to ``dataoffset`` and ``datasize`` are literals at parse time and properly take this into account in the optimizer.
 * Yul: Parse number literals for detecting duplicate switch cases.
 * Yul: Require switch cases to have the same type.


Build System:
 * Emscripten: Upgrade to emscripten 1.38.8 on travis and circleci.


### 0.5.2 (2018-12-19)

Language Features:
 * Control Flow Graph: Detect every access to uninitialized storage pointers.


Compiler Features:
 * Inline Assembly: Improve error messages around invalid function argument count.
 * Code Generator: Only check callvalue once if all functions are non-payable.
 * Code Generator: Use codecopy for string constants more aggressively.
 * Code Generator: Use binary search for dispatch function if more efficient. The size/speed tradeoff can be tuned using ``--optimize-runs``.
 * SMTChecker: Support mathematical and cryptographic functions in an uninterpreted way.
 * SMTChecker: Support one-dimensional mappings.
 * Standard JSON Interface: Disallow unknown keys in standard JSON input.
 * Standard JSON Interface: Only run code generation if it has been requested. This could lead to unsupported feature errors only being reported at the point where you request bytecode.
 * Static Analyzer: Do not warn about unused variables or state mutability for functions with an empty body.
 * Type Checker: Add an additional reason to be displayed when type conversion fails.
 * Yul: Support object access via ``datasize``, ``dataoffset`` and ``datacopy`` in standalone assembly mode.


Bugfixes:
 * Standard JSON Interface: Report specific error message for json input errors instead of internal compiler error.


Build System:
 * Replace the trusty PPA build by a static build on cosmic that is used for the trusty package instead.
 * Remove support for Visual Studio 2015.


### 0.5.1 (2018-12-03)

Language Features:
 * Allow mapping type for parameters and return variables of public and external library functions.
 * Allow public functions to override external functions.

Compiler Features:
 * Code generator: Do not perform redundant double cleanup on unsigned integers when loading from calldata.
 * Commandline interface: Experimental ``--optimize`` option for assembly mode (``--strict-assembly`` and ``--yul``).
 * SMTChecker: SMTLib2 queries and responses passed via standard JSON compiler interface.
 * SMTChecker: Support ``msg``, ``tx`` and ``block`` member variables.
 * SMTChecker: Support ``gasleft()`` and ``blockhash()`` functions.
 * SMTChecker: Support internal bound function calls.
 * Yul: Support Yul objects in ``--assemble``, ``--strict-assembly`` and ``--yul`` commandline options.

Bugfixes:
 * Assembly output: Do not mix in/out jump annotations with arguments.
 * Commandline interface: Fix crash when using ``--ast`` on empty runtime code.
 * Code Generator: Annotate jump from calldata decoder to function as "jump in".
 * Code Generator: Fix internal error related to state variables of function type access via base contract name.
 * Optimizer: Fix nondeterminism bug related to the boost version and constants representation. The bug only resulted in less optimal but still correct code because the generated routine is always verified to be correct.
 * Type Checker: Properly detect different return types when overriding an external interface function with a public contract function.
 * Type Checker: Disallow struct return types for getters of public state variables unless the new ABI encoder is active.
 * Type Checker: Fix internal compiler error when a field of a struct used as a parameter in a function type has a non-existent type.
 * Type Checker: Disallow functions ``sha3`` and ``suicide`` also without a function call.
 * Type Checker: Fix internal compiler error with ``super`` when base contract function is not implemented.
 * Type Checker: Fixed internal error when trying to create abstract contract in some cases.
 * Type Checker: Fixed internal error related to double declaration of events.
 * Type Checker: Disallow inline arrays of mapping type.
 * Type Checker: Consider abstract function to be implemented by public state variable.

Build System:
 * CMake: LLL is not built anymore by default. Must configure it with CMake as `-DLLL=ON`.
 * Docker: Includes both Scratch and Alpine images.
 * Emscripten: Upgrade to Emscripten SDK 1.37.21 and boost 1.67.

Solc-Js:
 * Fix handling of standard-json in the commandline executable.
 * Remove support of nodejs 4.


### 0.5.0 (2018-11-13)

How to update your code:
 * Change every ``.call()`` to a ``.call("")`` and every ``.call(signature, a, b, c)`` to use ``.call(abi.encodeWithSignature(signature, a, b, c))`` (the last one only works for value types).
 * Change every ``keccak256(a, b, c)`` to ``keccak256(abi.encodePacked(a, b, c))``.
 * Add ``public`` to every function and ``external`` to every fallback or interface function that does not specify its visibility already.
 * Make your fallback functions ``external``.
 * Explicitly state the data location for all variables of struct, array or mapping types (including function parameters), e.g. change ``uint[] x = m_x`` to ``uint[] storage x = m_x``. Note that ``external`` functions require parameters with a data location of ``calldata``.
 * Explicitly convert values of contract type to addresses before using an ``address`` member. Example: if ``c`` is a contract, change ``c.transfer(...)`` to ``address(c).transfer(...)``.
 * Declare variables and especially function arguments as ``address payable``, if you want to call ``transfer`` on them.

Breaking Changes:
 * ABI Encoder: Properly pad data from calldata (``msg.data`` and external function parameters). Use ``abi.encodePacked`` for unpadded encoding.
 * C API (``libsolc`` / raw ``soljson.js``): Removed the ``version``, ``license``, ``compileSingle``, ``compileJSON``, ``compileJSONCallback`` methods
   and replaced them with the ``solidity_license``, ``solidity_version`` and ``solidity_compile`` methods.
 * Code Generator: Signed right shift uses proper arithmetic shift, i.e. rounding towards negative infinity. Warning: this may silently change the semantics of existing code!
 * Code Generator: Revert at runtime if calldata is too short or points out of bounds. This is done inside the ``ABI decoder`` and therefore also applies to ``abi.decode()``.
 * Code Generator: Use ``STATICCALL`` for ``pure`` and ``view`` functions. This was already the case in the experimental 0.5.0 mode.
 * Commandline interface: Remove obsolete ``--formal`` option.
 * Commandline interface: Rename the ``--julia`` option to ``--yul``.
 * Commandline interface: Require ``-`` if standard input is used as source.
 * Commandline interface: Use hash of library name for link placeholder instead of name itself.
 * Compiler interface: Disallow remappings with empty prefix.
 * Control Flow Analyzer: Consider mappings as well when checking for uninitialized return values.
 * Control Flow Analyzer: Turn warning about returning uninitialized storage pointers into an error.
 * General: ``continue`` in a ``do...while`` loop jumps to the condition (it used to jump to the loop body). Warning: this may silently change the semantics of existing code.
 * General: Disallow declaring empty structs.
 * General: Disallow raw ``callcode`` (was already deprecated in 0.4.12). It is still possible to use it via inline assembly.
 * General: Disallow ``var`` keyword.
 * General: Disallow ``sha3`` and ``suicide`` aliases.
 * General: Disallow the ``throw`` statement. This was already the case in the experimental 0.5.0 mode.
 * General: Disallow the ``years`` unit denomination (was already deprecated in 0.4.24)
 * General: Introduce ``emit`` as a keyword instead of parsing it as identifier.
 * General: New keywords: ``calldata`` and ``constructor``
 * General: New reserved keywords: ``alias``, ``apply``, ``auto``, ``copyof``, ``define``, ``immutable``,
   ``implements``, ``macro``, ``mutable``, ``override``, ``partial``, ``promise``, ``reference``, ``sealed``,
   ``sizeof``, ``supports``, ``typedef`` and ``unchecked``.
 * General: Remove assembly instruction aliases ``sha3`` and ``suicide``
 * General: C99-style scoping rules are enforced now. This was already the case in the experimental 0.5.0 mode.
 * General: Disallow combining hex numbers with unit denominations (e.g. ``0x1e wei``). This was already the case in the experimental 0.5.0 mode.
 * JSON AST: Remove ``constant`` and ``payable`` fields (the information is encoded in the ``stateMutability`` field).
 * JSON AST: Replace the ``isConstructor`` field by a new ``kind`` field, which can be ``constructor``, ``fallback`` or ``function``.
 * Interface: Remove "clone contract" feature. The ``--clone-bin`` and ``--combined-json clone-bin`` commandline options are not available anymore.
 * Name Resolver: Do not exclude public state variables when looking for conflicting declarations.
 * Optimizer: Remove the no-op ``PUSH1 0 NOT AND`` sequence.
 * Parser: Disallow trailing dots that are not followed by a number.
 * Parser: Remove ``constant`` as function state mutability modifier.
 * Parser: Disallow uppercase X in hex number literals
 * Type Checker: Disallow assignments between tuples with different numbers of components. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow values for constants that are not compile-time constants. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow arithmetic operations for boolean variables.
 * Type Checker: Disallow tight packing of literals. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow calling base constructors without parentheses. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow conversions between ``bytesX`` and ``uintY`` of different size.
 * Type Checker: Disallow conversions between unrelated contract types. Explicit conversion via ``address`` can still achieve it.
 * Type Checker: Disallow empty return statements for functions with one or more return values.
 * Type Checker: Disallow empty tuple components. This was partly already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow multi-variable declarations with mismatching number of values. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow specifying base constructor arguments multiple times in the same inheritance hierarchy. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow calling constructor with wrong argument count. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow uninitialized storage variables. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Detecting cyclic dependencies in variables and structs is limited in recursion to 256.
 * Type Checker: Require explicit data location for all variables, including function parameters. This was partly already the case in the experimental 0.5.0 mode.
 * Type Checker: Only accept a single ``bytes`` type for ``.call()`` (and family), ``keccak256()``, ``sha256()`` and ``ripemd160()``.
 * Type Checker: Fallback function must be external. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Interface functions must be declared external. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Address members are not included in contract types anymore. An explicit conversion is now required before invoking an ``address`` member from a contract.
 * Type Checker: Disallow "loose assembly" syntax entirely. This means that jump labels, jumps and non-functional instructions cannot be used anymore.
 * Type System: Disallow explicit and implicit conversions from decimal literals to ``bytesXX`` types.
 * Type System: Disallow explicit and implicit conversions from hex literals to ``bytesXX`` types of different size.
 * Type System: Distinguish between payable and non-payable address types.
 * View Pure Checker: Disallow ``msg.value`` in (or introducing it via a modifier to) a non-payable function.
 * Remove obsolete ``std`` directory from the Solidity repository. This means accessing ``https://github.com/ethereum/solidity/blob/develop/std/*.sol`` (or ``https://github.com/ethereum/solidity/std/*.sol`` in Remix) will not be possible.
 * References Resolver: Turn missing storage locations into an error. This was already the case in the experimental 0.5.0 mode.
 * Syntax Checker: Disallow functions without implementation to use modifiers. This was already the case in the experimental 0.5.0 mode.
 * Syntax Checker: Named return values in function types are an error.
 * Syntax Checker: Strictly require visibility specifier for functions. This was already the case in the experimental 0.5.0 mode.
 * Syntax Checker: Disallow unary ``+``. This was already the case in the experimental 0.5.0 mode.
 * Syntax Checker: Disallow single statement variable declaration inside if/while/for bodies that are not blocks.
 * View Pure Checker: Strictly enforce state mutability. This was already the case in the experimental 0.5.0 mode.

Language Features:
 * General: Add ``staticcall`` to ``address``.
 * General: Allow appending ``calldata`` keyword to types, to explicitly specify data location for arguments of external functions.
 * General: Support ``pop()`` for storage arrays.
 * General: Scoping rules now follow the C99-style.
 * General: Allow ``enum``s in interfaces.
 * General: Allow ``mapping`` storage pointers as arguments and return values in all internal functions.
 * General: Allow ``struct``s in interfaces.
 * General: Provide access to the ABI decoder through ``abi.decode(bytes memory data, (...))``.
 * General: Disallow zero length for fixed-size arrays.
 * Parser: Accept the ``address payable`` type during parsing.

Compiler Features:
 * Build System: Support for Mojave version of macOS added.
 * Code Generator: ``CREATE2`` instruction has been updated to match EIP1014 (aka "Skinny CREATE2"). It also is accepted as part of Constantinople.
 * Code Generator: ``EXTCODEHASH`` instruction has been added based on EIP1052.
 * Type Checker: Nicer error message when trying to reference overloaded identifiers in inline assembly.
 * Type Checker: Show named argument in case of error.
 * Type System: IntegerType is split into IntegerType and AddressType internally.
 * Tests: Determine transaction status during IPC calls.
 * Code Generator: Allocate and free local variables according to their scope.
 * Removed ``pragma experimental "v0.5.0";``.
 * Syntax Checker: Improved error message for lookup in function types.
 * Name Resolver: Updated name suggestion look up function to take into account length of the identifier: 1: no search, 2-3: at most one change, 4-: at most two changes
 * SMTChecker: Support calls to internal functions that return none or a single value.

Bugfixes:
 * Build System: Support versions of CVC4 linked against CLN instead of GMP. In case of compilation issues due to the experimental SMT solver support, the solvers can be disabled when configuring the project with CMake using ``-DUSE_CVC4=OFF`` or ``-DUSE_Z3=OFF``.
 * Tests: Fix chain parameters to make ipc tests work with newer versions of cpp-ethereum.
 * Code Generator: Fix allocation of byte arrays (zeroed out too much memory).
 * Code Generator: Properly handle negative number literals in ABIEncoderV2.
 * Code Generator: Do not crash on using a length of zero for multidimensional fixed-size arrays.
 * Commandline Interface: Correctly handle paths with backslashes on windows.
 * Control Flow Analyzer: Ignore unimplemented functions when detecting uninitialized storage pointer returns.
 * Fix NatSpec json output for `@notice` and `@dev` tags on contract definitions.
 * Optimizer: Correctly estimate gas costs of constants for special cases.
 * Optimizer: Fix simplification rule initialization bug that appeared on some emscripten platforms.
 * References Resolver: Do not crash on using ``_slot`` and ``_offset`` suffixes on their own.
 * References Resolver: Enforce ``storage`` as data location for mappings.
 * References Resolver: Properly handle invalid references used together with ``_slot`` and ``_offset``.
 * References Resolver: Report error instead of assertion fail when FunctionType has an undeclared type as parameter.
 * References Resolver: Fix high CPU usage when using large variable names issue. Only suggest similar name if identifiers shorter than 80 characters.
 * Type Checker: Default data location for type conversions (e.g. from literals) is memory and not storage.
 * Type Checker: Disallow assignments to mappings within tuple assignments as well.
 * Type Checker: Disallow packed encoding of arrays of structs.
 * Type Checker: Allow assignments to local variables of mapping types.
 * Type Checker: Consider fixed size arrays when checking for recursive structs.
 * Type Checker: Fix crashes in erroneous tuple assignments in which the type of the right hand side cannot be determined.
 * Type Checker: Fix freeze for negative fixed-point literals very close to ``0``, such as ``-1e-100``.
 * Type Checker: Dynamic types as key for public mappings return error instead of assertion fail.
 * Type Checker: Fix internal error when array index value is too large.
 * Type Checker: Fix internal error when fixed-size array is too large to be encoded.
 * Type Checker: Fix internal error for array type conversions.
 * Type Checker: Fix internal error when array index is not an unsigned.
 * Type System: Allow arbitrary exponents for literals with a mantissa of zero.
 * Parser: Fix incorrect source location for nameless parameters.
 * Command Line Interface: Fix internal error when compiling stdin with no content and --ast option.


### 0.4.26 (2019-04-29)

Important Bugfixes:
 * Code Generator: Fix initialization routine of uninitialized internal function pointers in constructor context.
 * Type System: Use correct type name for contracts in event parameters when used in libraries. This affected code generation.

Bugfixes:
 * ABIEncoderV2: Refuse to generate code that is known to be potentially buggy.
 * General: Split rule list such that JavaScript environments with small stacks can use the compiler.

Note: The above changes are not included in 0.5.0, because they were backported.


### 0.4.25 (2018-09-12)

Important Bugfixes:
 * Code Generator: Properly perform cleanup for exponentiation and non-256 bit types.
 * Type Checker: Report error when using indexed structs in events with experimental ABIEncoderV2. This used to log wrong values.
 * Type Checker: Report error when using structs in events without experimental ABIEncoderV2. This used to crash or log the wrong values.
 * Parser: Consider all unicode line terminators (LF, VF, FF, CR, NEL, LS, PS) for single-line comments
   and string literals. They are invalid in strings and will end comments.
 * Parser: Disallow unterminated multi-line comments at the end of input.
 * Parser: Treat ``/** /`` as unterminated multi-line comment.

### 0.4.24 (2018-05-16)

Language Features:
 * Code Generator: Use native shift instructions on target Constantinople.
 * General: Allow multiple variables to be declared as part of a tuple assignment, e.g. ``(uint a, uint b) = ...``.
 * General: Remove deprecated ``constant`` as function state modifier from documentation and tests (but still leave it as a valid feature).
 * Type Checker: Deprecate the ``years`` unit denomination and raise a warning for it (or an error as experimental 0.5.0 feature).
 * Type Checker: Make literals (without explicit type casting) an error for tight packing as experimental 0.5.0 feature.
 * Type Checker: Warn about wildcard tuple assignments (this will turn into an error with version 0.5.0).
 * Type Checker: Warn when ``keccak256``, ``sha256`` and ``ripemd160`` are not used with a single bytes argument (suggest to use ``abi.encodePacked(...)``). This will turn into an error with version 0.5.0.

Compiler Features:
 * Build System: Update internal dependency of jsoncpp to 1.8.4, which introduces more strictness and reduces memory usage.
 * Control Flow Graph: Add Control Flow Graph as analysis structure.
 * Control Flow Graph: Warn about returning uninitialized storage pointers.
 * Gas Estimator: Only explore paths with higher gas costs. This reduces accuracy but greatly improves the speed of gas estimation.
 * Optimizer: Remove unnecessary masking of the result of known short instructions (``ADDRESS``, ``CALLER``, ``ORIGIN`` and ``COINBASE``).
 * Parser: Display nicer error messages by showing the actual tokens and not internal names.
 * Parser: Use the entire location of the token instead of only its starting position as source location for parser errors.
 * SMT Checker: Support state variables of integer and bool type.

Bugfixes:
 * Code Generator: Fix ``revert`` with reason coming from a state or local string variable.
 * Type Checker: Show proper error when trying to ``emit`` a non-event.
 * Type Checker: Warn about empty tuple components (this will turn into an error with version 0.5.0).
 * Type Checker: The ABI encoding functions are pure and thus can be used for constants.

### 0.4.23 (2018-04-19)

Features:
 * Build system: Support Ubuntu Bionic.
 * SMTChecker: Integration with CVC4 SMT solver
 * Syntax Checker: Warn about functions named "constructor".

Bugfixes:
 * Type Checker: Improve error message for failed function overload resolution.
 * Type Checker: Do not complain about new-style constructor and fallback function to have the same name.
 * Type Checker: Detect multiple constructor declarations in the new syntax and old syntax.
 * Type Checker: Explicit conversion of ``bytesXX`` to ``contract`` is properly disallowed.

### 0.4.22 (2018-04-16)

Features:
 * Code Generator: Initialize arrays without using ``msize()``.
 * Code Generator: More specialized and thus optimized implementation for ``x.push(...)``
 * Commandline interface: Error when missing or inaccessible file detected. Suppress it with the ``--ignore-missing`` flag.
 * Constant Evaluator: Fix evaluation of single element tuples.
 * General: Add encoding routines ``abi.encodePacked``, ``abi.encode``, ``abi.encodeWithSelector`` and ``abi.encodeWithSignature``.
 * General: Add global function ``gasleft()`` and deprecate ``msg.gas``.
 * General: Add global function ``blockhash(uint)`` and deprecate ``block.hash(uint)``.
 * General: Allow providing reason string for ``revert()`` and ``require()``.
 * General: Introduce new constructor syntax using the ``constructor`` keyword as experimental 0.5.0 feature.
 * General: Limit the number of errors output in a single run to 256.
 * General: Support accessing dynamic return data in post-byzantium EVMs.
 * General: Allow underscores in numeric and hex literals to separate thousands and quads.
 * Inheritance: Error when using empty parentheses for base class constructors that require arguments as experimental 0.5.0 feature.
 * Inheritance: Error when using no parentheses in modifier-style constructor calls as experimental 0.5.0 feature.
 * Interfaces: Allow overriding external functions in interfaces with public in an implementing contract.
 * Optimizer: Optimize ``SHL`` and ``SHR`` only involving constants (Constantinople only).
 * Optimizer: Remove useless ``SWAP1`` instruction preceding a commutative instruction (such as ``ADD``, ``MUL``, etc).
 * Optimizer: Replace comparison operators (``LT``, ``GT``, etc) with opposites if preceded by ``SWAP1``, e.g. ``SWAP1 LT`` is replaced with ``GT``.
 * Optimizer: Optimize across ``mload`` if ``msize()`` is not used.
 * Static Analyzer: Error on duplicated super constructor calls as experimental 0.5.0 feature.
 * Syntax Checker: Issue warning for empty structs (or error as experimental 0.5.0 feature).
 * Syntax Checker: Warn about modifiers on functions without implementation (this will turn into an error with version 0.5.0).
 * Syntax Tests: Add source locations to syntax test expectations.
 * Type Checker: Improve documentation and warnings for accessing contract members inherited from ``address``.

Bugfixes:
 * Code Generator: Allow ``block.blockhash`` without being called.
 * Code Generator: Do not include internal functions in the runtime bytecode which are only referenced in the constructor.
 * Code Generator: Properly skip unneeded storage array cleanup when not reducing length.
 * Code Generator: Bugfix in modifier lookup in libraries.
 * Code Generator: Implement packed encoding of external function types.
 * Code Generator: Treat empty base constructor argument list as not provided.
 * Code Generator: Properly force-clean bytesXX types for shortening conversions.
 * Commandline interface: Fix error messages for imported files that do not exist.
 * Commandline interface: Support ``--evm-version constantinople`` properly.
 * DocString Parser: Fix error message for empty descriptions.
 * Gas Estimator: Correctly ignore costs of fallback function for other functions.
 * JSON AST: Remove storage qualifier for type name strings.
 * Parser: Fix internal compiler error when parsing ``var`` declaration without identifier.
 * Parser: Fix parsing of getters for function type variables.
 * Standard JSON: Support ``constantinople`` as ``evmVersion`` properly.
 * Static Analyzer: Fix non-deterministic order of unused variable warnings.
 * Static Analyzer: Invalid arithmetic with constant expressions causes errors.
 * Type Checker: Fix detection of recursive structs.
 * Type Checker: Fix asymmetry bug when comparing with literal numbers.
 * Type System: Improve error message when attempting to shift by a fractional amount.
 * Type System: Make external library functions accessible.
 * Type System: Prevent encoding of weird types.
 * Type System: Restrict rational numbers to 4096 bits.

### 0.4.21 (2018-03-07)

Features:
 * Code Generator: Assert that ``k != 0`` for ``mulmod(a, b, k)`` and ``addmod(a, b, k)`` as experimental 0.5.0 feature.
 * Code Generator: Do not retain any gas in calls (except if EVM version is set to homestead).
 * Code Generator: Use ``STATICCALL`` opcode for calling ``view`` and ``pure`` functions as experimental 0.5.0 feature.
 * General: C99/C++-style scoping rules (instead of JavaScript function scoping) take effect as experimental v0.5.0 feature.
 * General: Improved messaging when error spans multiple lines of a sourcefile
 * General: Support and recommend using ``emit EventName();`` to call events explicitly.
 * Inline Assembly: Enforce strict mode as experimental 0.5.0 feature.
 * Interface: Provide ability to select target EVM version (homestead or byzantium, with byzantium being the default).
 * Standard JSON: Reject badly formatted invalid JSON inputs.
 * Type Checker: Disallow uninitialized storage pointers as experimental 0.5.0 feature.
 * Syntax Analyser: Do not warn about experimental features if they do not concern code generation.
 * Syntax Analyser: Do not warn about ``pragma experimental "v0.5.0"`` and do not set the experimental flag in the bytecode for this.
 * Syntax Checker: Mark ``throw`` as an error as experimental 0.5.0 feature.
 * Syntax Checker: Issue error if no visibility is specified on contract functions as experimental 0.5.0 feature.
 * Syntax Checker: Issue warning when using overloads of ``address`` on contract instances.
 * Type Checker: disallow combining hex numbers and unit denominations as experimental 0.5.0 feature.

Bugfixes:
 * Assembly: Raise error on oversized number literals in assembly.
 * JSON-AST: Add "documentation" property to function, event and modifier definition.
 * Resolver: Properly determine shadowing for imports with aliases.
 * Standalone Assembly: Do not ignore input after closing brace of top level block.
 * Standard JSON: Catch errors properly when invalid "sources" are passed.
 * Standard JSON: Ensure that library addresses supplied are of correct length and hex prefixed.
 * Type Checker: Properly detect which array and struct types are unsupported by the old ABI encoder.
 * Type Checker: Properly warn when using ``_offset`` and ``_slot`` for constants in inline assembly.
 * Commandline interface: throw error if option is unknown

### 0.4.20 (2018-02-14)

Features:
 * Code Generator: Prevent non-view functions in libraries from being called
   directly (as opposed to via delegatecall).
 * Commandline interface: Support strict mode of assembly (disallowing jumps,
   instructional opcodes, etc) with the ``--strict-assembly`` switch.
 * Inline Assembly: Issue warning for using jump labels (already existed for jump instructions).
 * Inline Assembly: Support some restricted tokens (return, byte, address) as identifiers in Iulia mode.
 * Optimiser: Replace ``x % 2**i`` by ``x & (2**i-1)``.
 * Resolver: Continue resolving references after the first error.
 * Resolver: Suggest alternative identifiers if a given identifier is not found.
 * SMT Checker: Take if-else branch conditions into account in the SMT encoding of the program
   variables.
 * Syntax Checker: Deprecate the ``var`` keyword (and mark it an error as experimental 0.5.0 feature).
 * Type Checker: Allow `this.f.selector` to be a pure expression.
 * Type Checker: Issue warning for using ``public`` visibility for interface functions.
 * Type Checker: Limit the number of warnings raised for creating abstract contracts.

Bugfixes:
 * Error Output: Truncate huge number literals in the middle to avoid output blow-up.
 * Parser: Disallow event declarations with no parameter list.
 * Standard JSON: Populate the ``sourceLocation`` field in the error list.
 * Standard JSON: Properly support contract and library file names containing a colon (such as URLs).
 * Type Checker: Suggest the experimental ABI encoder if using ``struct``s as function parameters
   (instead of an internal compiler error).
 * Type Checker: Improve error message for wrong struct initialization.

### 0.4.19 (2017-11-30)

Features:
 * Code Generator: New ABI decoder which supports structs and arbitrarily nested
   arrays and checks input size (activate using ``pragma experimental ABIEncoderV2;``).
 * General: Allow constant variables to be used as array length.
 * Inline Assembly: ``if`` statement.
 * Standard JSON: Support the ``outputSelection`` field for selective compilation of target artifacts.
 * Syntax Checker: Turn the usage of ``callcode`` into an error as experimental 0.5.0 feature.
 * Type Checker: Improve address checksum warning.
 * Type Checker: More detailed errors for invalid array lengths (such as division by zero).

Bugfixes:

### 0.4.18 (2017-10-18)

Features:
 * Code Generator: Always use all available gas for calls as experimental 0.5.0 feature
   (previously, some amount was retained in order to work in pre-Tangerine-Whistle
   EVM versions)
 * Parser: Better error message for unexpected trailing comma in parameter lists.
 * Standard JSON: Support the ``outputSelection`` field for selective compilation of supplied sources.
 * Syntax Checker: Unary ``+`` is now a syntax error as experimental 0.5.0 feature.
 * Type Checker: Disallow non-pure constant state variables as experimental 0.5.0 feature.
 * Type Checker: Do not add members of ``address`` to contracts as experimental 0.5.0 feature.
 * Type Checker: Force interface functions to be external as experimental 0.5.0 feature.
 * Type Checker: Require ``storage`` or ``memory`` keyword for local variables as experimental 0.5.0 feature.
 * Compiler Interface: Better formatted error message for long source snippets

Bugfixes:
 * Code Generator: Allocate one byte per memory byte array element instead of 32.
 * Code Generator: Do not accept data with less than four bytes (truncated function
   signature) for regular function calls - fallback function is invoked instead.
 * Optimizer: Remove unused stack computation results.
 * Parser: Fix source location of VariableDeclarationStatement.
 * Type Checker: Allow ``gas`` in view functions.
 * Type Checker: Do not mark event parameters as shadowing state variables.
 * Type Checker: Prevent duplicate event declarations.
 * Type Checker: Properly check array length and don't rely on an assertion in code generation.
 * Type Checker: Properly support overwriting members inherited from ``address`` in a contract
   (such as ``balance``, ``transfer``, etc.)
 * Type Checker: Validate each number literal in tuple expressions even if they are not assigned from.

### 0.4.17 (2017-09-21)

Features:
 * Assembly Parser: Support multiple assignment (``x, y := f()``).
 * Code Generator: Keep a single copy of encoding functions when using the experimental "ABIEncoderV2".
 * Code Generator: Partial support for passing ``structs`` as arguments and return parameters (requires ``pragma experimental ABIEncoderV2;`` for now).
 * General: Support ``pragma experimental "v0.5.0";`` to activate upcoming breaking changes.
 * General: Added ``.selector`` member on external function types to retrieve their signature.
 * Optimizer: Add new optimization step to remove unused ``JUMPDEST``s.
 * Static Analyzer: Warn when using deprecated builtins ``sha3`` and ``suicide``
   (replaced by ``keccak256`` and ``selfdestruct``, introduced in 0.4.2 and 0.2.0, respectively).
 * Syntax Checker: Warn if no visibility is specified on contract functions.
 * Type Checker: Display helpful warning for unused function arguments/return parameters.
 * Type Checker: Do not show the same error multiple times for events.
 * Type Checker: Greatly reduce the number of duplicate errors shown for duplicate constructors and functions.
 * Type Checker: Warn on using literals as tight packing parameters in ``keccak256``, ``sha3``, ``sha256`` and ``ripemd160``.
 * Type Checker: Enforce ``view`` and ``pure``.
 * Type Checker: Enforce ``view`` / ``constant`` with error as experimental 0.5.0 feature.
 * Type Checker: Enforce fallback functions to be ``external`` as experimental 0.5.0 feature.

Bugfixes:
 * ABI JSON: Include all overloaded events.
 * Parser: Crash fix related to parseTypeName.
 * Type Checker: Allow constant byte arrays.

### 0.4.16 (2017-08-24)

Features:
 * ABI JSON: Include new field ``stateMutability`` with values ``pure``, ``view``,
   ``nonpayable`` and ``payable``.
 * Analyzer: Experimental partial support for Z3 SMT checker ("SMTChecker").
 * Build System: Shared libraries (``libsolutil``, ``libevmasm``, ``libsolidity``
   and ``liblll``) are no longer produced during the build process.
 * Code generator: Experimental new implementation of ABI encoder that can
   encode arbitrarily nested arrays ("ABIEncoderV2")
 * Metadata: Store experimental flag in metadata CBOR.
 * Parser: Display previous visibility specifier in error if multiple are found.
 * Parser: Introduce ``pure`` and ``view`` keyword for functions,
   ``constant`` remains an alias for ``view`` and pureness is not enforced yet,
   so use with care.
 * Static Analyzer: Warn about large storage structures.
 * Syntax Checker: Support ``pragma experimental <feature>;`` to turn on
   experimental features.
 * Type Checker: More detailed error message for invalid overrides.
 * Type Checker: Warn about shifting a literal.

Bugfixes:
 * Assembly Parser: Be more strict about number literals.
 * Assembly Parser: Limit maximum recursion depth.
 * Parser: Enforce commas between array and tuple elements.
 * Parser: Limit maximum recursion depth.
 * Type Checker: Crash fix related to ``using``.
 * Type Checker: Disallow constructors in libraries.
 * Type Checker: Reject the creation of interface contracts using the ``new`` statement.

### 0.4.15 (2017-08-08)

Features:
 * Type Checker: Show unimplemented function if trying to instantiate an abstract class.

Bugfixes:
 * Code Generator: ``.delegatecall()`` should always return execution outcome.
 * Code Generator: Provide "new account gas" for low-level ``callcode`` and ``delegatecall``.
 * Type Checker: Constructors must be implemented if declared.
 * Type Checker: Disallow the ``.gas()`` modifier on ``ecrecover``, ``sha256`` and ``ripemd160``.
 * Type Checker: Do not mark overloaded functions as shadowing other functions.
 * Type Checker: Internal library functions must be implemented if declared.

### 0.4.14 (2017-07-31)

Features:
 * C API (``jsonCompiler``): Export the ``license`` method.
 * Code Generator: Optimise the fallback function, by removing a useless jump.
 * Inline Assembly: Show useful error message if trying to access calldata variables.
 * Inline Assembly: Support variable declaration without initial value (defaults to 0).
 * Metadata: Only include files which were used to compile the given contract.
 * Type Checker: Disallow value transfers to contracts without a payable fallback function.
 * Type Checker: Include types in explicit conversion error message.
 * Type Checker: Raise proper error for arrays too large for ABI encoding.
 * Type checker: Warn if using ``this`` in a constructor.
 * Type checker: Warn when existing symbols, including builtins, are overwritten.

Bugfixes:
 * Code Generator: Properly clear return memory area for ecrecover.
 * Type Checker: Fix crash for some assignment to non-lvalue.
 * Type Checker: Fix invalid "specify storage keyword" warning for reference members of structs.
 * Type Checker: Mark modifiers as internal.
 * Type Checker: Re-allow multiple mentions of the same modifier per function.


### 0.4.13 (2017-07-06)

Features:
 * Syntax Checker: Deprecated "throw" in favour of require(), assert() and revert().
 * Type Checker: Warn if a local storage reference variable does not explicitly use the keyword ``storage``.

Bugfixes:
 * Code Generator: Correctly unregister modifier variables.
 * Compiler Interface: Only output AST if analysis was successful.
 * Error Output: Do not omit the error type.

### 0.4.12 (2017-07-03)

Features:
 * Assembly: Add ``CREATE2`` (EIP86), ``STATICCALL`` (EIP214), ``RETURNDATASIZE`` and ``RETURNDATACOPY`` (EIP211) instructions.
 * Assembly: Display auxiliary data in the assembly output.
 * Assembly: Renamed ``SHA3`` to ``KECCAK256``.
 * AST: export all attributes to JSON format.
 * C API (``jsonCompiler``): Use the Standard JSON I/O internally.
 * Code Generator: Added the Whiskers template system.
 * Inline Assembly: ``for`` and ``switch`` statements.
 * Inline Assembly: Function definitions and function calls.
 * Inline Assembly: Introduce ``keccak256`` as an opcode. ``sha3`` is still a valid alias.
 * Inline Assembly: Present proper error message when not supplying enough arguments to a functional
   instruction.
 * Inline Assembly: Warn when instructions shadow Solidity variables.
 * Inline Assembly: Warn when using ``jump``s.
 * Remove obsolete Why3 output.
 * Type Checker: Enforce strict UTF-8 validation.
 * Type Checker: Warn about copies in storage that might overwrite unexpectedly.
 * Type Checker: Warn about type inference from literal numbers.
 * Static Analyzer: Warn about deprecation of ``callcode``.

Bugfixes:
 * Assembly: mark ``MLOAD`` to have side effects in the optimiser.
 * Code Generator: Fix ABI encoding of empty literal string.
 * Code Generator: Fix negative stack size checks.
 * Code generator: Use ``REVERT`` instead of ``INVALID`` for generated input validation routines.
 * Inline Assembly: Enforce function arguments when parsing functional instructions.
 * Optimizer: Disallow optimizations involving ``MLOAD`` because it changes ``MSIZE``.
 * Static Analyzer: Unused variable warnings no longer issued for variables used inside inline assembly.
 * Type Checker: Fix address literals not being treated as compile-time constants.
 * Type Checker: Fixed crash concerning non-callable types.
 * Type Checker: Fixed segfault with constant function parameters
 * Type Checker: Disallow comparisons between mapping and non-internal function types.
 * Type Checker: Disallow invoking the same modifier multiple times.
 * Type Checker: Do not treat strings that look like addresses as addresses.
 * Type Checker: Support valid, but incorrectly rejected UTF-8 sequences.

### 0.4.11 (2017-05-03)

Features:
 * Implement the Standard JSON Input / Output API
 * Support ``interface`` contracts.
 * C API (``jsonCompiler``): Add the ``compileStandard()`` method to process a Standard JSON I/O.
 * Commandline interface: Add the ``--standard-json`` parameter to process a Standard JSON I/O.
 * Commandline interface: Support ``--allow-paths`` to define trusted import paths. Note: the
   path(s) of the supplied source file(s) is always trusted.
 * Inline Assembly: Storage variable access using ``_slot`` and ``_offset`` suffixes.
 * Inline Assembly: Disallow blocks with unbalanced stack.
 * Static analyzer: Warn about statements without effects.
 * Static analyzer: Warn about unused local variables, parameters, and return parameters.
 * Syntax checker: issue deprecation warning for unary '+'

Bugfixes:
 * Assembly output: Implement missing AssemblyItem types.
 * Compiler interface: Fix a bug where source indexes could be inconsistent between Solidity compiled
   with different compilers (clang vs. gcc) or compiler settings. The bug was visible in AST
   and source mappings.
 * Gas Estimator: Reflect the most recent fee schedule.
 * Type system: Contract inheriting from base with unimplemented constructor should be abstract.
 * Optimizer: Number representation bug in the constant optimizer fixed.

### 0.4.10 (2017-03-15)

Features:
 * Add ``assert(condition)``, which throws if condition is false (meant for internal errors).
 * Add ``require(condition)``, which throws if condition is false (meant for invalid input).
 * Commandline interface: Do not overwrite files unless forced.
 * Introduce ``.transfer(value)`` for sending Ether.
 * Code generator: Support ``revert()`` to abort with rolling back, but not consuming all gas.
 * Inline assembly: Support ``revert`` (EIP140) as an opcode.
 * Parser: Support scientific notation in numbers (e.g. ``2e8`` and ``200e-2``).
 * Type system: Support explicit conversion of external function to address.
 * Type system: Warn if base of exponentiation is literal (result type might be unexpected).
 * Type system: Warn if constant state variables are not compile-time constants.

Bugfixes:
 * Commandline interface: Always escape filenames (replace ``/``, ``:`` and ``.`` with ``_``).
 * Commandline interface: Do not try creating paths ``.`` and ``..``.
 * Commandline interface: Allow long library names.
 * Parser: Disallow octal literals.
 * Type system: Fix a crash caused by continuing on fatal errors in the code.
 * Type system: Disallow compound assignment for tuples.
 * Type system: Detect cyclic dependencies between constants.
 * Type system: Disallow arrays with negative length.
 * Type system: Fix a crash related to invalid binary operators.
 * Type system: Disallow ``var`` declaration with empty tuple type.
 * Type system: Correctly convert function argument types to pointers for member functions.
 * Type system: Move privateness of constructor into AST itself.
 * Inline assembly: Charge one stack slot for non-value types during analysis.
 * Assembly output: Print source location before the operation it refers to instead of after.
 * Optimizer: Stop trying to optimize tricky constants after a while.

### 0.4.9 (2017-01-31)

Features:
 * Compiler interface: Contracts and libraries can be referenced with a ``file:`` prefix to make them unique.
 * Compiler interface: Report source location for "stack too deep" errors.
 * AST: Use deterministic node identifiers.
 * Inline assembly: introduce ``invalid`` (EIP141) as an opcode.
 * Type system: Introduce type identifier strings.
 * Type checker: Warn about invalid checksum for addresses and deduce type from valid ones.
 * Metadata: Do not include platform in the version number.
 * Metadata: Add option to store sources as literal content.
 * Code generator: Extract array utils into low-level functions.
 * Code generator: Internal errors (array out of bounds, etc.) now cause a reversion by using an invalid
   instruction (0xfe - EIP141) instead of an invalid jump. Invalid jump is still kept for explicit throws.

Bugfixes:
 * Code generator: Allow recursive structs.
 * Inline assembly: Disallow variables named like opcodes.
 * Type checker: Allow multiple events of the same name (but with different arities or argument types)
 * Natspec parser: Fix error with ``@param`` parsing and whitespace.

### 0.4.8 (2017-01-13)

Features:
 * Optimiser: Performance improvements.
 * Output: Print assembly in new standardized Solidity assembly format.

Bugfixes:
 * Remappings: Prefer longer context over longer prefix.
 * Type checker, code generator: enable access to events of base contracts' names.
 * Imports: ``import ".dir/a"`` is not a relative path.  Relative paths begin with directory ``.`` or ``..``.
 * Type checker, disallow inheritances of different kinds (e.g. a function and a modifier) of members of the same name

### 0.4.7 (2016-12-15)

Features:
 * Bitshift operators.
 * Type checker: Warn when ``msg.value`` is used in non-payable function.
 * Code generator: Inject the Swarm hash of a metadata file into the bytecode.
 * Code generator: Replace expensive memcpy precompile by simple assembly loop.
 * Optimizer: Some dead code elimination.

Bugfixes:
 * Code generator: throw if calling the identity precompile failed during memory (array) copying.
 * Type checker: string literals that are not valid UTF-8 cannot be converted to string type
 * Code generator: any non-zero value given as a boolean argument is now converted into 1.
 * AST Json Converter: replace ``VariableDefinitionStatement`` nodes with ``VariableDeclarationStatement``
 * AST Json Converter: fix the camel case in ``ElementaryTypeNameExpression``
 * AST Json Converter: replace ``public`` field with ``visibility`` in the function definition nodes

### 0.4.6 (2016-11-22)

Bugfixes:
 * Optimizer: Knowledge about state was not correctly cleared for JUMPDESTs (introduced in 0.4.5)

### 0.4.5 (2016-11-21)

Features:
 * Function types
 * Do-while loops: support for a ``do <block> while (<expr>);`` control structure
 * Inline assembly: support ``invalidJumpLabel`` as a jump label.
 * Type checker: now more eagerly searches for a common type of an inline array with mixed types
 * Code generator: generates a runtime error when an out-of-range value is converted into an enum type.

Bugfixes:

 * Inline assembly: calculate stack height warning correctly even when local variables are used.
 * Code generator: check for value transfer in non-payable constructors.
 * Parser: disallow empty enum definitions.
 * Type checker: disallow conversion between different enum types.
 * Interface JSON: do not include trailing new line.

### 0.4.4 (2016-10-31)

Bugfixes:
 * Type checker: forbid signed exponential that led to an incorrect use of EXP opcode.
 * Code generator: properly clean higher order bytes before storing in storage.

### 0.4.3 (2016-10-25)

Features:

 * Inline assembly: support both ``suicide`` and ``selfdestruct`` opcodes
   (note: ``suicide`` is deprecated).
 * Inline assembly: issue warning if stack is not balanced after block.
 * Include ``keccak256()`` as an alias to ``sha3()``.
 * Support shifting constant numbers.

Bugfixes:
 * Commandline interface: Disallow unknown options in ``solc``.
 * Name resolver: Allow inheritance of ``enum`` definitions.
 * Type checker: Proper type checking for bound functions.
 * Type checker: fixed crash related to invalid fixed point constants
 * Type checker: fixed crash related to invalid literal numbers.
 * Type checker: ``super.x`` does not look up ``x`` in the current contract.
 * Code generator: expect zero stack increase after ``super`` as an expression.
 * Code generator: fix an internal compiler error for ``L.Foo`` for ``enum Foo`` defined in library ``L``.
 * Code generator: allow inheritance of ``enum`` definitions.
 * Inline assembly: support the ``address`` opcode.
 * Inline assembly: fix parsing of assignment after a label.
 * Inline assembly: external variables of unsupported type (such as ``this``, ``super``, etc.)
   are properly detected as unusable.
 * Inline assembly: support variables within modifiers.
 * Optimizer: fix related to stale knowledge about SHA3 operations

### 0.4.2 (2016-09-17)

Bugfixes:

 * Code Generator: Fix library functions being called from payable functions.
 * Type Checker: Fixed a crash about invalid array types.
 * Code Generator: Fixed a call gas bug that became visible after
   version 0.4.0 for calls where the output is larger than the input.

### 0.4.1 (2016-09-09)

 * Build System: Fixes to allow library compilation.

### 0.4.0 (2016-09-08)

This release deliberately breaks backwards compatibility mostly to
enforce some safety features. The most important change is that you have
to explicitly specify if functions can receive ether via the ``payable``
modifier. Furthermore, more situations cause exceptions to be thrown.

Minimal changes to be made for upgrade:
 - Add ``payable`` to all functions that want to receive Ether
   (including the constructor and the fallback function).
 - Change ``_`` to ``_;`` in modifiers.
 - Add version pragma to each file: ``pragma solidity ^0.4.0;``

Breaking Changes:

 * Source files have to specify the compiler version they are
   compatible with using e.g. ``pragma solidity ^0.4.0;`` or
   ``pragma solidity >=0.4.0 <0.4.8;``
 * Functions that want to receive Ether have to specify the
   new ``payable`` modifier (otherwise they throw).
 * Contracts that want to receive Ether with a plain "send"
   have to implement a fallback function with the ``payable``
   modifier. Contracts now throw if no payable fallback
   function is defined and no function matches the signature.
 * Failing contract creation through "new" throws.
 * Division / modulus by zero throws.
 * Function call throws if target contract does not have code
 * Modifiers are required to contain ``_`` (use ``if (false) _`` as a workaround if needed).
 * Modifiers: return does not skip part in modifier after ``_``.
 * Placeholder statement `_` in modifier now requires explicit `;`.
 * ``ecrecover`` now returns zero if the input is malformed (it previously returned garbage).
 * The ``constant`` keyword cannot be used for constructors or the fallback function.
 * Removed ``--interface`` (Solidity interface) output option
 * JSON AST: General cleanup, renamed many nodes to match their C++ names.
 * JSON output: ``srcmap-runtime`` renamed to ``srcmapRuntime``.
 * Moved (and reworked) standard library contracts from inside the compiler to github.com/ethereum/solidity/std
   (``import "std";`` or ``import owned;`` do not work anymore).
 * Confusing and undocumented keyword ``after`` was removed.
 * New reserved words: ``abstract``, ``hex``, ``interface``, ``payable``, ``pure``, ``static``, ``view``.

Features:

 * Hexadecimal string literals: ``hex"ab1248fe"``
 * Internal: Inline assembly usable by the code generator.
 * Commandline interface: Using ``-`` as filename allows reading from stdin.
 * Interface JSON: Fallback function is now part of the ABI.
 * Interface: Version string now *SemVer* compatible.
 * Code generator: Do not provide "new account gas" if we know the called account exists.

Bugfixes:

 * JSON AST: Nodes were added at wrong parent
 * Why3 translator: Crash fix for exponentiation
 * Commandline Interface: linking libraries with underscores in their name.
 * Type Checker: Fallback function cannot return data anymore.
 * Code Generator: Fix crash when ``sha3()`` was used on unsupported types.
 * Code Generator: Manually set gas stipend for ``.send(0)``.

Lots of changes to the documentation mainly by voluntary external contributors.

### 0.3.6 (2016-08-10)

Features:

 * Formal verification: Take external effects on a contract into account.
 * Type Checker: Warning about unused return value of low-level calls and send.
 * Output: Source location and node id as part of AST output
 * Output: Source location mappings for bytecode
 * Output: Formal verification as part of json compiler output.

Bugfixes:

 * Commandline Interface: Do not crash if input is taken from stdin.
 * Scanner: Correctly support unicode escape codes in strings.
 * JSON output: Fix error about relative / absolute source file names.
 * JSON output: Fix error about invalid utf8 strings.
 * Code Generator: Dynamic allocation of empty array caused infinite loop.
 * Code Generator: Correctly calculate gas requirements for memcpy precompile.
 * Optimizer: Clear known state if two code paths are joined.

### 0.3.5 (2016-06-10)

Features:

 * Context-dependent path remappings (different modules can use the same library in different versions)

Bugfixes:

 * Type Checking: Dynamic return types were removed when fetching data from external calls, now they are replaced by an "unusable" type.
 * Type Checking: Overrides by constructors were considered making a function non-abstract.

### 0.3.4 (2016-05-31)

No change outside documentation.

### 0.3.3 (2016-05-27)

 * Allow internal library functions to be called (by "inlining")
 * Fractional/rational constants (only usable with fixed point types, which are still in progress)
 * Inline assembly has access to internal functions (as jump labels)
 * Running `solc` without arguments on a terminal will print help.
 * Bugfix: Remove some non-determinism in code generation.
 * Bugfix: Corrected usage of not / bnot / iszero in inline assembly
 * Bugfix: Correctly clean bytesNN types before comparison

### 0.3.2 (2016-04-18)

 * Bugfix: Inline assembly parser: `byte` opcode was unusable
 * Bugfix: Error reporting: tokens for variably-sized types were not converted to string properly
 * Bugfix: Dynamic arrays of structs were not deleted correctly.
 * Bugfix: Static arrays in constructor parameter list were not decoded correctly.

### 0.3.1 (2016-03-31)

 * Inline assembly
 * Bugfix: Code generation: array access with narrow types did not clean higher order bits
 * Bugfix: Error reporting: error reporting with unknown source location caused a crash

### 0.3.0 (2016-03-11)

BREAKING CHANGES:

 * Added new keywords `assembly`, `foreign`, `fixed`, `ufixed`, `fixedNxM`, `ufixedNxM` (for various values of M and N), `timestamp`
 * Number constant division does not round to integer, but to a fixed point type (e.g. `1 / 2 != 1`, but `1 / 2 == 0.5`).
 * Library calls now default to use DELEGATECALL (e.g. called library functions see the same value as the calling function for `msg.value` and `msg.sender`).
 * `<address>.delegatecall` as a low-level calling interface

Bugfixes:
 * Fixed a bug in the optimizer that resulted in comparisons being wrong.


### 0.2.2 (2016-02-17)

 * Index access for types `bytes1`, ..., `bytes32` (only read access for now).
 * Bugfix: Type checker crash for wrong number of base constructor parameters.

### 0.2.1 (2016-01-30)

 * Inline arrays, i.e. `var y = [1,x,f()];` if there is a common type for `1`, `x` and `f()`. Note that the result is always a fixed-length memory array and conversion to dynamic-length memory arrays is not yet possible.
 * Import similar to ECMAScript6 import (`import "abc.sol" as d` and `import {x, y} from "abc.sol"`).
 * Commandline compiler solc automatically resolves missing imports and allows for "include directories".
 * Conditional: `x ? y : z`
 * Bugfix: Fixed several bugs where the optimizer generated invalid code.
 * Bugfix: Enums and structs were not accessible to other contracts.
 * Bugfix: Fixed segfault connected to function parameter types, appeared during gas estimation.
 * Bugfix: Type checker crash for wrong number of base constructor parameters.
 * Bugfix: Allow function overloads with different array types.
 * Bugfix: Allow assignments of type `(x) = 7`.
 * Bugfix: Type `uint176` was not available.
 * Bugfix: Fixed crash during type checking concerning constructor calls.
 * Bugfix: Fixed crash during code generation concerning invalid accessors for struct types.
 * Bugfix: Fixed crash during code generating concerning computing a hash of a struct type.

### 0.2.0 (2015-12-02)

 * **Breaking Change**: `new ContractName.value(10)()` has to be written as `(new ContractName).value(10)()`
 * Added `selfdestruct` as an alias for `suicide`.
 * Allocation of memory arrays using `new`.
 * Binding library functions to types via `using x for y`
 * `addmod` and `mulmod` (modular addition and modular multiplication with arbitrary intermediate precision)
 * Bugfix: Constructor arguments of fixed array type were not read correctly.
 * Bugfix: Memory allocation of structs containing arrays or strings.
 * Bugfix: Data location for explicit memory parameters in libraries was set to storage.

### 0.1.7 (2015-11-17)

 * Improved error messages for unexpected tokens.
 * Proof-of-concept transcompilation to why3 for formal verification of contracts.
 * Bugfix: Arrays (also strings) as indexed parameters of events.
 * Bugfix: Writing to elements of `bytes` or `string` overwrite others.
 * Bugfix: "Successor block not found" on Windows.
 * Bugfix: Using string literals in tuples.
 * Bugfix: Cope with invalid commit hash in version for libraries.
 * Bugfix: Some test framework fixes on windows.

### 0.1.6 (2015-10-16)

 * `.push()` for dynamic storage arrays.
 * Tuple expressions (`(1,2,3)` or `return (1,2,3);`)
 * Declaration and assignment of multiple variables (`var (x,y,) = (1,2,3,4,5);` or `var (x,y) = f();`)
 * Destructuring assignment (`(x,y,) = (1,2,3)`)
 * Bugfix: Internal error about usage of library function with invalid types.
 * Bugfix: Correctly parse `Library.structType a` at statement level.
 * Bugfix: Correctly report source locations of parenthesized expressions (as part of "tuple" story).

### 0.1.5 (2015-10-07)

 * Breaking change in storage encoding: Encode short byte arrays and strings together with their length in storage.
 * Report warnings
 * Allow storage reference types for public library functions.
 * Access to types declared in other contracts and libraries via `.`.
 * Version stamp at beginning of runtime bytecode of libraries.
 * Bugfix: Problem with initialized string state variables and dynamic data in constructor.
 * Bugfix: Resolve dependencies concerning `new` automatically.
 * Bugfix: Allow four indexed arguments for anonymous events.
 * Bugfix: Detect too large integer constants in functions that accept arbitrary parameters.

### 0.1.4 (2015-09-30)

 * Bugfix: Returning fixed-size arrays.
 * Bugfix: combined-json output of solc.
 * Bugfix: Accessing fixed-size array return values.
 * Bugfix: Disallow assignment from literal strings to storage pointers.
 * Refactoring: Move type checking into its own module.

### 0.1.3 (2015-09-25)

 * `throw` statement.
 * Libraries that contain functions which are called via CALLCODE.
 * Linker stage for compiler to insert other contract's addresses (used for libraries).
 * Compiler option to output runtime part of contracts.
 * Compile-time out of bounds check for access to fixed-size arrays by integer constants.
 * Version string includes libevmasm/libethereum's version (contains the optimizer).
 * Bugfix: Accessors for constant public state variables.
 * Bugfix: Propagate exceptions in clone contracts.
 * Bugfix: Empty single-line comments are now treated properly.
 * Bugfix: Properly check the number of indexed arguments for events.
 * Bugfix: Strings in struct constructors.

### 0.1.2 (2015-08-20)

 * Improved commandline interface.
 * Explicit conversion between `bytes` and `string`.
 * Bugfix: Value transfer used in clone contracts.
 * Bugfix: Problem with strings as mapping keys.
 * Bugfix: Prevent usage of some operators.

### 0.1.1 (2015-08-04)

 * Strings can be used as mapping keys.
 * Clone contracts.
 * Mapping members are skipped for structs in memory.
 * Use only a single stack slot for storage references.
 * Improved error message for wrong argument count. (#2456)
 * Bugfix: Fix comparison between `bytesXX` types. (#2087)
 * Bugfix: Do not allow floats for integer literals. (#2078)
 * Bugfix: Some problem with many local variables. (#2478)
 * Bugfix: Correctly initialise `string` and `bytes` state variables.
 * Bugfix: Correctly compute gas requirements for callcode.

### 0.1.0 (2015-07-10)
