function uSuffix(uint8) pure returns (int) {}
function uSuffix(uint16) pure returns (int) {}

contract C {
    int a = 127 uSuffix; // TODO: Error should say it's ambiguous
}
// ----
// TypeError 2144: (119-130): No matching declaration found after variable lookup.
