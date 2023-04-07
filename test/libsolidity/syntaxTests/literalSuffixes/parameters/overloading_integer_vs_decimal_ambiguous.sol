function suffix(uint) pure suffix returns (bool) {}
function suffix(uint, uint) pure suffix returns (address) {}

contract C {
    function f() public pure {
        int a = 1 suffix;
    }
}
// ----
// TypeError 2144: (176-182): No matching declaration found after variable lookup.
