function suffix(uint) pure suffix returns (int) {}
function suffix(uint, uint) pure suffix returns (int) {}

contract C {
    function f() public pure {
        int a = 1 suffix;
    }
}
// ----
// TypeError 4487: (169-177): No unique declaration found after argument-dependent lookup.
