function iuSuffix(uint8) pure returns (int) {}
function iuSuffix(int8) pure returns (int) {}

contract C {
    int a = 127 iuSuffix;
}
// ----
// TypeError 4487: (119-131): No unique declaration found after argument-dependent lookup.
