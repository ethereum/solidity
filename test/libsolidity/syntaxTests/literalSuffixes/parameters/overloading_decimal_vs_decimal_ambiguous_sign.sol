function iuSuffix(uint8, uint) pure suffix returns (int) {}
function iuSuffix(int8, uint) pure suffix returns (int) {}

contract C {
    uint a = 1.27 iuSuffix;
}
// ----
// TypeError 2144: (151-159): No matching declaration found after variable lookup.
