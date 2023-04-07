function iuSuffix(uint8) pure suffix returns (int) {}
function iuSuffix(int8) pure suffix returns (int) {}

contract C {
    int a = 127 iuSuffix;
}
// ----
// TypeError 2144: (137-145): No matching declaration found after variable lookup.
