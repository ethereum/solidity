function uSuffix(uint8, uint) pure returns (uint) {}
function uSuffix(uint16, uint) pure returns (uint) {}

contract C {
    uint a = 1.27 uSuffix;
}
// ----
// TypeError 4487: (134-146): No unique declaration found after argument-dependent lookup.
