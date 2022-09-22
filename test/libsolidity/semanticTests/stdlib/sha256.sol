pragma stdlib;

import { sha256 } from "std/cryptography.sol";

contract C {
    function f(uint256 a) external returns (bytes32) {
        return sha256(abi.encodePacked(a));
    }
}

// ====
// EVMVersion: >=constantinople
// ----
// f(uint256): 1 -> 0xec4916dd28fc4c10d78e287ca5d9cc51ee1ae73cbfde08c6b37324cbfaac8bc5
