pragma stdlib;

import { ripemd160 } from "std/cryptography.sol";

contract C {
    function f(uint256 a) external returns (bytes20) {
        return ripemd160(abi.encodePacked(a));
    }
}

// ====
// EVMVersion: >=constantinople
// ----
// f(uint256): 4 -> 0x1b0f3c404d12075c68c938f9f60ebea4f74941a0000000000000000000000000
