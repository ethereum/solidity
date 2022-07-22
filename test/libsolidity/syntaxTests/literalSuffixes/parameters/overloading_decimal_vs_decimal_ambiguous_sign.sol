function iuSuffix(uint8, uint) pure returns (uint) {}
function iuSuffix(int8, uint) pure returns (uint) {}

contract C {
    uint a = 1.27 iuSuffix;
}
// ----
// TypeError 4487: (134-147): No unique declaration found after argument-dependent lookup.
