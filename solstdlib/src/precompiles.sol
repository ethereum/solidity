// SPDX-License-Identifier: Apache-2.0

function sha256(bytes memory input) returns (bytes32 ret) {
  assembly {
    let success := staticcall(gas(), 2, add(input, 32), mload(input), 0, 32)
    if iszero(success) { revert(0, 0) }
    ret := mload(0)
  }
}

function ripemd160(bytes memory input) returns (bytes20 ret) {
  assembly {
    let success := staticcall(gas(), 3, add(input, 32), mload(input), 0, 32)
    if iszero(success) { revert(0, 0) }
    // TODO: check byteorder
    ret := mload(0)
  }
}

function ecrecover(bytes32 hash, uint8 v, bytes32 r, bytes32 s) returns (address addr) {
  assembly {
    // Prepare input
    // We are using the free memory pointer.
    let input := mload(0x40)
    mstore(input, hash)
    mstore(add(input, 32), v)
    mstore(add(input, 64), r)
    mstore(add(input, 96), s)

    // Prepare output
    mstore(0, 0)

    // Call the precompile
    let ret := staticcall(gas(), 1, input, 128, 0, 32)
    if iszero(ret) { revert(0, 0) }
    addr := mload(0)
  }
}
