function iuSuffix(uint8) pure returns (int) {}
function iuSuffix(int8) pure returns (int) {}

contract C {
    int a = 127 iuSuffix; // TODO: Error should say it's ambiguous
}
// ----
// TypeError 2144: (119-131): No matching declaration found after variable lookup.
