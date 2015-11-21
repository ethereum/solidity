---
layout: docs
title: Miscellaneous
permalink: /docs/miscellaneous/
---

## Layout of State Variables in Storage

Statically-sized variables (everything except mapping and dynamically-sized array types) are laid out contiguously in storage starting from position `0`. Multiple items that need less than 32 bytes are packed into a single storage slot if possible, according to the following rules:

- The first item in a storage slot is stored lower-order aligned.
- Elementary types use only that many bytes that are necessary to store them.
- If an elementary type does not fit the remaining part of a storage slot, it is moved to the next storage slot.
- Structs and array data always start a new slot and occupy whole slots (but items inside a struct or array are packed tightly according to these rules).

The elements of structs and arrays are stored after each other, just as if they were given explicitly.

Due to their unpredictable size, mapping and dynamically-sized array types use a `sha3`
computation to find the starting position of the value or the array data. These starting positions are always full stack slots.

The mapping or the dynamic array itself
occupies an (unfilled) slot in storage at some position `p` according to the above rule (or by
recursively applying this rule for mappings to mappings or arrays of arrays). For a dynamic array, this slot stores the number of elements in the array (byte arrays and strings are an exception here, see below). For a mapping, the slot is unused (but it is needed so that two equal mappings after each other will use a different hash distribution).
Array data is located at `sha3(p)` and the value corresponding to a mapping key
`k` is located at `sha3(k . p)` where `.` is concatenation. If the value is again a
non-elementary type, the positions are found by adding an offset of `sha3(k . p)`.

`bytes` and `string` store their data in the same slot where also the length is stored if they are short. In particular: If the data is at most `31` bytes long, it is stored in the higher-order bytes (left aligned) and the lowest-order byte stores `length * 2`. If it is longer, the main slot stores `length * 2 + 1` and the data is stored as usual in `sha3(slot)`.

So for the following contract snippet:
{% highlight javascript %}
contract c {
  struct S { uint a; uint b; }
  uint x;
  mapping(uint => mapping(uint => S)) data;
}
{% endhighlight %}
The position of `data[4][9].b` is at `sha3(uint256(9) . sha3(uint256(4) . uint256(1))) + 1`.

## Esoteric Features

There are some types in Solidity's type system that have no counterpart in the syntax. One of these types are the types of functions. But still, using `var` it is possible to have local variables of these types:

{% highlight javascript %}
contract FunctionSelector {
  function select(bool useB, uint x) returns (uint z) {
    var f = a;
    if (useB) f = b;
    return f(x);
  }
  function a(uint x) returns (uint z) {
    return x * x;
  }
  function b(uint x) returns (uint z) {
    return 2 * x;
  }
}
{% endhighlight %}

Calling `select(false, x)` will compute `x * x` and `select(true, x)` will compute `2 * x`.


## Internals - the Optimizer

The Solidity optimizer operates on assembly, so it can be and also is used by other languages. It splits the sequence of instructions into basic blocks at JUMPs and JUMPDESTs. Inside these blocks, the instructions are analysed and every modification to the stack, to memory or storage is recorded as an expression which consists of an instruction and a list of arguments which are essentially pointers to other expressions. The main idea is now to find expressions that are always equal (on every input) and combine them into an expression class. The optimizer first tries to find each new expression in a list of already known expressions. If this does not work, the expression is simplified according to rules like `constant` + `constant` = `sum_of_constants` or `X` * 1 = `X`. Since this is done recursively, we can also apply the latter rule if the second factor is a more complex expression where we know that it will always evaluate to one. Modifications to storage and memory locations have to erase knowledge about storage and memory locations which are not known to be different: If we first write to location x and then to location y and both are input variables, the second could overwrite the first, so we actually do not know what is stored at x after we wrote to y. On the other hand, if a simplification of the expression x - y evaluates to a non-zero constant, we know that we can keep our knowledge about what is stored at x.

At the end of this process, we know which expressions have to be on the stack in the end and have a list of modifications to memory and storage. This information is stored together with the basic blocks and is used to link them. Furthermore, knowledge about the stack, storage and memory configuration is forwarded to the next block(s). If we know the targets of all JUMP and JUMPI instructions, we can build a complete control flow graph of the program. If there is only one target we do not know (this can happen as in principle, jump targets can be computed from inputs), we have to erase all knowledge about the input state of a block as it can be the target of the unknown JUMP. If a JUMPI is found whose condition evaluates to a constant, it is transformed to an unconditional jump.

As the last step, the code in each block is completely re-generated. A dependency graph is created from the expressions on the stack at the end of the block and every operation that is not part of this graph is essentially dropped. Now code is generated that applies the modifications to memory and storage in the order they were made in the original code (dropping modifications which were found not to be needed) and finally, generates all values that are required to be on the stack in the correct place.

These steps are applied to each basic block and the newly generated code is used as replacement if it is smaller. If a basic block is split at a JUMPI and during the analysis, the condition evaluates to a constant, the JUMPI is replaced depending on the value of the constant, and thus code like
{% highlight javascript %}
var x = 7;
data[7] = 9;
if (data[x] != x + 2)
  return 2;
else
  return 1;
{% endhighlight %}
is simplified to code which can also be compiled from
{% highlight javascript %}
data[7] = 9;
return 1;
{% endhighlight %}
even though the instructions contained a jump in the beginning.

## Using the Commandline Compiler

One of the build targets of the Solidity repository is `solc`, the solidity commandline compiler.
Using `solc --help` provides you with an explanation of all options. The compiler can produce various outputs, ranging from simple binaries and assembly over an abstract syntax tree (parse tree) to estimations of gas usage.
If you only want to compile a single file, you run it as `solc --bin sourceFile.sol` and it will print the binary. Before you deploy your contract, activate the optimizer while compiling using `solc --optimize --bin sourceFile.sol`. If you want to get some of the more advanced output variants of `solc`, it is probably better to tell it to output everything to separate files using `solc -o outputDirectory --bin --ast --asm sourceFile.sol`.

Of course, you can also specify several source files and actually that is also required if you use the `import` statement in Solidity: The compiler will (for now) not automatically discover source files for you, so you have to provide it with all source files your project consists of.

If your contracts use [libraries](#libraries), you will notice that the bytecode contains substrings of the form `__LibraryName______`. You can use `solc` as a linker meaning that it will insert the library addresses for you at those points:

Either add `--libraries "Math:0x12345678901234567890 Heap:0xabcdef0123456" to your command to provide an address for each library or store the string in a file (one library per line) and run `solc` using `--libraries fileName`.

If `solc` is called with the option `--link`, all input files are interpreted to be unlinked binaries (hex-encoded) in the `__LibraryName____`-format given above and are linked in-place (if the input is read from stdin, it is written to stdout). All options except `--libraries` are ignored (including `-o`) in this case.

## Tips and Tricks

 * Use `delete` on arrays to delete all its elements.
 * Use shorter types for struct elements and sort them such that short types are grouped together. This can lower the gas costs as multiple SSTORE operations might be combined into a single (SSTORE costs 5000 or 20000 gas, so this is what you want to optimise). Use the gas price estimator (with optimiser enabled) to check!
 * Make your state variables public - the compiler will create [getters](#accessor-functions) for you for free.
 * If you end up checking conditions on input or state a lot at the beginning of your functions, try using [modifiers](#function-modifiers)
 * If your contract has a function called `send` but you want to use the built-in send-function, use `address(contractVariable).send(amount)`.
 * If you want your contracts to receive ether when called via `send`, you have to implement the [fallback function](#fallback-functions).
 * Initialise storage structs with a single assignment: `x = MyStruct({a: 1, b: 2});`

## Pitfalls

Unfortunately, there are some subtleties the compiler does not yet warn you about.

 - If you use `StructName x` or `uint[] x` as a local variable, it **has** to be assigned from a state variable,
   otherwise it behaves like a "null pointer" to storage, so you cannot use it on its own.
   Please read about [data locations](#data-location). The most common solution is to use `StructName memory x` or `uint[] memory x`.
 - In `for (var i = 0; i < arrayName.length; i++) { ... }`, the type of `i` will be `uint8`, because this is the smallest type that is required to hold the value `0`. If the array has more than 255 elements, the loop will not terminate.

## Cheatsheet

### Global Variables

 - `block.coinbase` (`address`): current block miner's address
 - `block.difficulty` (`uint`): current block difficulty
 - `block.gaslimit` (`uint`): current block gaslimit
 - `block.number` (`uint`): current block number
 - `block.blockhash` (`function(uint) returns (bytes32)`): hash of the given block - only works for 256 most recent blocks
 - `block.timestamp` (`uint`): current block timestamp
 - `msg.data` (`bytes`): complete calldata
 - `msg.gas` (`uint`): remaining gas
 - `msg.sender` (`address`): sender of the message (current call)
 - `msg.value` (`uint`): number of wei sent with the message
 - `now` (`uint`): current block timestamp (alias for `block.timestamp`)
 - `tx.gasprice` (`uint`): gas price of the transaction
 - `tx.origin` (`address`): sender of the transaction (full call chain)
 - `sha3(...) returns (bytes32)`: compute the Ethereum-SHA3 hash of the (tightly packed) arguments
 - `sha256(...) returns (bytes32)`: compute the SHA256 hash of the (tightly packed) arguments
 - `ripemd160(...) returns (bytes20)`: compute RIPEMD of 256 the (tightly packed) arguments
 - `ecrecover(bytes32, byte, bytes32, bytes32) returns (address)`: recover public key from elliptic curve signature
 - `addmod(uint x, uint y, uint k) returns (uint)`: compute `(x + y) % k` where the addition is performed with arbitrary precision and does not wrap around at `2**256`.
 - `mulmod(uint x, uint y, uint k) returns (uint)`: compute `(x * y) % k` where the multiplication is performed with arbitrary precision and does not wrap around at `2**256`.
 - `this` (current contract's type): the current contract, explicitly convertible to `address`
 - `super`: the contract one level higher in the inheritance hierarchy
 - `suicide(address)`: suicide the current contract, sending its funds to the given address
 - `<address>.balance`: balance of the address in Wei
 - `<address>.send(uint256) returns (bool)`: send given amount of Wei to address, returns `false` on failure.

### Function Visibility Specifiers

{% highlight javascript %}
function myFunction() <visibility specifier> returns (bool) {
    return true;
}
{% endhighlight %}

 - `public`: visible externally and internally (creates accessor function for storage/state variables)
 - `private`: only visible in the current contract
 - `external`: only visible externally (only for functions) - i.e. can only be message-called (via `this.fun`)
 - `internal`: only visible internally

### Modifiers

 - `constant` for state variables: Disallows assignment (except initialisation), does not occupy storage slot.
 - `constant` for functions: Disallows modification of state - this is not enforced yet.
 - `anonymous` for events: Does not store event signature as topic.
 - `indexed` for event parameters: Stores the parameter as topic.

### Types

TODO
