function iuSuffix(uint8, uint) pure suffix returns (uint) {}
function iuSuffix(int8, uint) pure suffix returns (uint) {}

contract C {
    uint a = 1.27 iuSuffix;
}
// ----
// TypeError 4487: (148-161): No unique declaration found after argument-dependent lookup.
