function suffix(uint) pure returns (int) {}
function suffix(uint, uint) pure returns (int) {}

contract C {
    function f() public pure {
        int a = 1 suffix; // TODO: Error should say it's ambiguous
    }
}
// ----
// TypeError 2144: (155-163): No matching declaration found after variable lookup.
