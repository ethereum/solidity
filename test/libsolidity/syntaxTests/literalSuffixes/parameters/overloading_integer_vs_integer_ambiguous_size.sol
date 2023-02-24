function uSuffix(uint8) pure suffix returns (int) {}
function uSuffix(uint16) pure suffix returns (int) {}

contract C {
    int a = 127 uSuffix;
}
// ----
// TypeError 4487: (133-144): No unique declaration found after argument-dependent lookup.
