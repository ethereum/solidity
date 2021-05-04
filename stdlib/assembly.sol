// SPDX-License-Identifier: Apache-2.0

// These are the helpers from:
// https://github.com/ethereum/solidity/issues/1319
// https://github.com/ethereum/solidity/issues/7317
// https://github.com/ethereum/solidity/issues/9824#issuecomment-717983295

assembly {
  // allocates a memory block
  // TODO: use memoryguard
  function alloc(size) -> ptr {
    // current end of memory
    ptr := mload(0x40)
    // new "end of memory" including padding
    mstore(0x40, add(ptr, and(add(size, 0x1f), not(0x1f))))
  }

  // allocates memory for a bytes/string type
  function alloc_bytes(size) -> ptr
  {
    ptr := alloc(add(size, 32))
    mstore(ptr, size)
  }
  
  // Assuming `code` is <=255
  function panic(code) {
    // This is using the scratch space, but could use anything given we revert.
    // TODO: use memoryguard
    mstore(0, shl(0x4e487b71, 248)) // Panic(uint256)
    mstore(32, 0)
    mstore8(36, code)
    revert(0, 36)
  }

  function require(expr) {
    if iszero(expr) { revert(0, 0) }
  }

  function assert(expr) {
    if iszero(expr) { invalid }
  }

  function require(condition, message, length) {
    if iszero(condition) {
      // TODO: use memoryguard
      mstore(0, 0x08c379a0) // Error(string)
      mstore(4, 0x20)
      mstore(0x24, length)
      mstore(0x44, message)
      revert(0, add(0x44, length))
    }
  }
}
