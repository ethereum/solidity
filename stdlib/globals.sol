// SPDX-License-Identifier: Apache-2.0

//import ecrecover from std;
//import sha256 from std;
//import chainid from std;

contract C{

function addmod(uint x, uint y, uint k) public returns (uint r) {
  require(k != 0);
  assembly {
    r := addmod(x, y, k)
  }
}

function mulmod(uint x, uint y, uint k) public returns (uint r) {
  require(k != 0);
  assembly {
    r := mulmod(x, y, k)
  }
}

function keccak256(bytes memory input) public returns (bytes32 ret) {
  assembly {
    ret := keccak256(add(input, 32), mload(input))
  }
}

function sha256(bytes memory input) public returns (bytes32 ret) {
  assembly {
    let success := staticcall(gas(), 2, add(input, 32), mload(input), 0, 32)
    if iszero(success) { revert(0, 0) }
    ret := mload(0)
  }
}

function ripemd160(bytes memory input) public returns (bytes20 ret) {
  assembly {
    let success := staticcall(gas(), 3, add(input, 32), mload(input), 0, 32)
    if iszero(success) { revert(0, 0) }
    // TODO: check byteorder
    ret := mload(0)
  }
}

function ecrecover(bytes32 hash, uint8 v, bytes32 r, bytes32 s) public returns (address addr) {
  assembly {
    // Prepare input
    // We are using the free memory pointer.
    let input := mload(0x40)
    mstore(input, hash)
    mstore(add(input, 32), v)
    mstore(add(input, 64), r)
    mstore(add(input, 96), s)

    // Prepare output
    let output := add(input, 128)
    mstore(output, 0)

    // Call the precompile
    let ret := staticcall(gas(), 1, input, 128, output, 32)
    switch ret
    case 1 { // Success
      addr := mload(output)
    }
    default { // Failure
      // Need to do anything?
    }
  }
}

function selfdestruct(address payable recipient) public {
  assembly {
    selfdestruct(recipient)
  }
}

}
