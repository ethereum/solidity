function suffix256(uint) pure suffix returns (bool) {}
function suffix256(uint, uint) pure suffix returns (address) {}

contract C {
    address a = 1.1 suffix256;
}
// ----
// TypeError 2144: (153-162): No matching declaration found after variable lookup.
