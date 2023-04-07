==== Source: A.sol ====
function s(uint, uint) pure suffix returns (uint) {}
==== Source: B.sol ====
import {s} from "A.sol";

function s(string memory) pure returns (string memory) {}

contract C {
    function run() public pure {
        1024 s;
    }
}
// ----
// TypeError 2144: (B.sol:144-145): No matching declaration found after variable lookup.
