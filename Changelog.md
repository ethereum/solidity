### 0.5.0 (unreleased)

How to update your code:
 * Change every ``.call()`` to a ``.call("")`` and every ``.call(signature, a, b, c)`` to use ``.call(abi.encodeWithSignature(signature, a, b, c))`` (the last one only works for value types).
 * Change every ``keccak256(a, b, c)`` to ``keccak256(abi.encodePacked(a, b, c))``.
 * Make your fallback functions ``external``.
 * Explicitly state the storage location for local variables of struct and array types, e.g. change ``uint[] x = m_x`` to ``uint[] storage x = m_x``.


Breaking Changes:
 * ABI Encoder: Properly pad data from calldata (``msg.data`` and external function parameters). Use ``abi.encodePacked`` for unpadded encoding.
 * Code Generator: Signed right shift uses proper arithmetic shift, i.e. rounding towards negative infinity. Warning: this may silently change the semantics of existing code!
 * Code Generator: Revert at runtime if calldata is too short or points out of bounds. This is done inside the ``ABI decoder`` and therefore also applies to ``abi.decode()``.
 * Code Generator: Use ``STATICCALL`` for ``pure`` and ``view`` functions. This was already the case in the experimental 0.5.0 mode.
 * Commandline interface: Remove obsolete ``--formal`` option.
 * Commandline interface: Rename the ``--julia`` option to ``--yul``.
 * Commandline interface: Require ``-`` if standard input is used as source.
 * General: ``continue`` in a ``do...while`` loop jumps to the condition (it used to jump to the loop body). Warning: this may silently change the semantics of existing code.
 * General: Disallow declaring empty structs.
 * General: Disallow raw ``callcode`` (was already deprecated in 0.4.12). It is still possible to use it via inline assembly.
 * General: Disallow ``var`` keyword.
 * General: Disallow ``sha3`` and ``suicide`` aliases.
 * General: Disallow the ``years`` unit denomination (was already deprecated in 0.4.24)
 * General: Introduce ``emit`` as a keyword instead of parsing it as identifier.
 * General: New keywords: ``calldata``
 * General: New reserved keywords: ``alias``, ``apply``, ``auto``, ``copyof``, ``define``, ``immutable``,
   ``implements``, ``macro``, ``mutable``, ``override``, ``partial``, ``promise``, ``reference``, ``sealed``,
   ``sizeof``, ``supports``, ``typedef`` and ``unchecked``.
 * General: Remove assembly instruction aliases ``sha3`` and ``suicide``
 * General: C99-style scoping rules are enforced now. This was already the case in the experimental 0.5.0 mode.
 * General: Disallow combining hex numbers with unit denominations (e.g. ``0x1e wei``). This was already the case in the experimental 0.5.0 mode.
 * Optimizer: Remove the no-op ``PUSH1 0 NOT AND`` sequence.
 * Parser: Disallow trailing dots that are not followed by a number.
 * Parser: Remove ``constant`` as function state mutability modifer.
 * Type Checker: Disallow assignments between tuples with different numbers of components. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow values for constants that are not compile-time constants. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow arithmetic operations for boolean variables.
 * Type Checker: Disallow tight packing of literals. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow conversions between ``bytesX`` and ``uintY`` of different size.
 * Type Checker: Disallow empty tuple components. This was partly already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow multi-variable declarations with mismatching number of values. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow specifying base constructor arguments multiple times in the same inheritance hierarchy. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow calling constructor with wrong argument count. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Disallow uninitialized storage variables. This was already the case in the experimental 0.5.0 mode.
 * Type Checker: Only accept a single ``bytes`` type for ``.call()`` (and family), ``keccak256()``, ``sha256()`` and ``ripemd160()``.
 * Type Checker: Fallback function must be external. This was already the case in the experimental 0.5.0 mode.
 * Remove obsolete ``std`` directory from the Solidity repository. This means accessing ``https://github.com/ethereum/soldity/blob/develop/std/*.sol`` (or ``https://github.com/ethereum/solidity/std/*.sol`` in Remix) will not be possible.
 * References Resolver: Turn missing storage locations into an error. This was already the case in the experimental 0.5.0 mode.
 * Syntax Checker: Named return values in function types are an error.
 * Syntax Checker: Disallow unary ``+``. This was already the case in the experimental 0.5.0 mode.
 * View Pure Checker: Strictly enfore state mutability. This was already the case in the experimental 0.5.0 mode.

Language Features:
 * General: Allow appending ``calldata`` keyword to types, to explicitly specify data location for arguments of external functions.
 * General: Support ``pop()`` for storage arrays.
 * General: Scoping rules now follow the C99-style.

Compiler Features:
 * C API (``libsolc``): Export the ``solidity_license``, ``solidity_version`` and ``solidity_compile`` methods.
 * Type Checker: Show named argument in case of error.
 * Tests: Determine transaction status during IPC calls.
 * Code Generator: Allocate and free local variables according to their scope.

Bugfixes:
 * Tests: Fix chain parameters to make ipc tests work with newer versions of cpp-ethereum.
 * Code Generator: Fix allocation of byte arrays (zeroed out too much memory).
 * Fix NatSpec json output for `@notice` and `@dev` tags on contract definitions.
 * Type Checker: Consider fixed size arrays when checking for recursive structs.
 * Type System: Allow arbitrary exponents for literals with a mantissa of zero.

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
 * Code Generator: Use ``STATICCALL`` opcode for calling ``view`` and ``pure`` functions as experimenal 0.5.0 feature.
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
 * Build System: Shared libraries (``libdevcore``, ``libevmasm``, ``libsolidity``
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
 * Interface: Version string now *semver* compatible.
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
