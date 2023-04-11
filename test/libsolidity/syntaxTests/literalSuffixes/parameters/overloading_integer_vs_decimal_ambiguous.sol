function suffix(uint) pure suffix returns (bool) {}
function suffix(uint, uint) pure suffix returns (address) {}

contract C {
    function f() public pure {
        int a = 1 suffix;
    }
}
// ----
// TypeError 4487: (176-182): No unique declaration found after argument-dependent lookup.
