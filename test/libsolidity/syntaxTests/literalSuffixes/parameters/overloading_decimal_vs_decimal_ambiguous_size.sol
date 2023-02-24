function uSuffix(uint8, uint) pure suffix returns (uint) {}
function uSuffix(uint16, uint) pure suffix returns (uint) {}

contract C {
    uint a = 1.27 uSuffix;
}
// ----
// TypeError 4487: (148-160): No unique declaration found after argument-dependent lookup.
