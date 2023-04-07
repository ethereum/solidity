==== Source: A.sol ====
function s(uint, uint) pure suffix returns (uint) {}

function f(uint, uint) pure returns (uint) {}

==== Source: B.sol ====
import "A.sol";

function s(string memory) pure returns (string memory) {}

function f(string memory) pure suffix returns (string memory) {}

contract C {
    function run() public pure {
        1024 s;
        "a" f;
    }
}
==== Source: C.sol ====
import {s, f} from "A.sol";

function s(string memory) pure returns (string memory) {}

function f(string memory) pure suffix returns (string memory) {}

contract D {
    function run() public pure {
        1024 s;
        "a" f;
    }
}
// ----
// TypeError 2144: (B.sol:201-202): No matching declaration found after variable lookup.
