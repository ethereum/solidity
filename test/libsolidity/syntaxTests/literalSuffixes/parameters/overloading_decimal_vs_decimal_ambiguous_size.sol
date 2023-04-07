function uSuffix(uint8, uint) pure suffix returns (int) {}
function uSuffix(uint16, uint) pure suffix returns (int) {}

contract C {
    uint a = 1.27 uSuffix;
}
// ----
// TypeError 2144: (151-158): No matching declaration found after variable lookup.
