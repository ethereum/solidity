### 0.4.12 (unreleased)

Features:
 * AST: export all attributes to Json format
 * Inline Assembly: Present proper error message when not supplying enough arguments to a functional
   instruction.

Bugfixes:
 * Unused variable warnings no longer issued for variables used inside inline assembly

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
 * Bugfix: Fixed segfault connected to function paramater types, appeared during gas estimation.
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
