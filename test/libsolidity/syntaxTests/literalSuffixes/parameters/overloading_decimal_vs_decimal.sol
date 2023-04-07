function uSuffix(uint8, uint) pure suffix returns (uint) {}
function uSuffix(uint16, uint) pure suffix returns (int) {}

contract C {
    int a = 1.024 uSuffix;
}
// ----
// TypeError 2144: (152-159): No matching declaration found after variable lookup.
