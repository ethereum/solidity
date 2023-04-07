function uSuffix(uint8) pure suffix returns (uint) {}
function uSuffix(uint16) pure suffix returns (int) {}

contract C {
    int a = 1024 uSuffix;
}
// ----
// TypeError 2144: (139-146): No matching declaration found after variable lookup.
