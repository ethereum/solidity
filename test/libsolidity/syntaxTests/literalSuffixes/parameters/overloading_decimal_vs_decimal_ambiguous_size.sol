function uSuffix(uint8, uint) pure suffix returns (int) {}
function uSuffix(uint16, uint) pure suffix returns (int) {}

contract C {
    uint a = 1.27 uSuffix;
}
// ----
// TypeError 4487: (151-158): No unique declaration found after argument-dependent lookup.
