function suffix(uint) pure returns (int) {}
function suffix(uint, uint) pure returns (int) {}

contract C {
    function f() public pure {
        int a = 1 suffix;
    }
}
// ----
// TypeError 4487: (155-163): No unique declaration found after argument-dependent lookup.
