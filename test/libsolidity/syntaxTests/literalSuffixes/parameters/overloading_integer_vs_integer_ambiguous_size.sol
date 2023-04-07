function uSuffix(uint8) pure suffix returns (int) {}
function uSuffix(uint16) pure suffix returns (int) {}

contract C {
    int a = 127 uSuffix;
}
// ----
// TypeError 2144: (137-144): No matching declaration found after variable lookup.
