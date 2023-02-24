function iuSuffix(uint8) pure suffix returns (int) {}
function iuSuffix(int8) pure suffix returns (int) {}

contract C {
    int a = 127 iuSuffix;
}
// ----
// TypeError 4487: (133-145): No unique declaration found after argument-dependent lookup.
