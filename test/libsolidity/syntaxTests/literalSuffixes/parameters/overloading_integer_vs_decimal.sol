function suffix256(uint) pure returns (int) {}
function suffix256(uint, uint) pure returns (int) {}

function suffix8(uint) pure returns (int) {}
function suffix8(uint8, uint) pure returns (int) {}

contract C {
    function f() public pure {
        int a = 1.1 suffix256;  // TODO: Should match only (uint, uint)
        int b = 1024 suffix8;   // TODO: Should match only uint
    }
}
// ----
// TypeError 2144: (259-272): No matching declaration found after variable lookup.
