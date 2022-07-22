function uSuffix(uint8) pure returns (int) {}
function uSuffix(uint16) pure returns (int) {}

contract C {
    int a = 127 uSuffix;
}
// ----
// TypeError 4487: (119-130): No unique declaration found after argument-dependent lookup.
