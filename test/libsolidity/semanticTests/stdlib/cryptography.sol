pragma stdlib;

import "std/cryptography.sol";

contract C {

    function f(uint256 a) external returns (bytes32) {
        return sha256(abi.encodePacked(a));
    }

    function g(uint256 a) external returns (bytes20) {
        return ripemd160(abi.encodePacked(a));
    }

    function h(bytes32 h, uint8 v, bytes32 r, bytes32 s) public returns (address addr) {
        return ecrecover(h, v, r, s);
    }
}

// ====
// EVMVersion: >=constantinople
// ----
// f(uint256): 1 -> 0xec4916dd28fc4c10d78e287ca5d9cc51ee1ae73cbfde08c6b37324cbfaac8bc5
// g(uint256): 4 -> 0x1b0f3c404d12075c68c938f9f60ebea4f74941a0000000000000000000000000
// h(bytes32,uint8,bytes32,bytes32):
// 0x18c547e4f7b0f325ad1e56f57e26c745b09a3e503d86e00e5255ff7f715d3d1c,
// 28,
// 0x73b1693892219d736caba55bdb67216e485557ea6b6af75f37096c9aa6a5a75f,
// 0xeeb940b1d03b21e36b0e47e79769f095fe2ab855bd91e3a38756b7d75a9c4549
// -> 0xa94f5374fce5edbc8e2a8697c15331677e6ebf0b
