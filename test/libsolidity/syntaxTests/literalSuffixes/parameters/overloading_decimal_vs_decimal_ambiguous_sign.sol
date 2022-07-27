function iuSuffix(uint8, uint) pure returns (uint) {}
function iuSuffix(int8, uint) pure returns (uint) {}

contract C {
    uint a = 1.27 iuSuffix; // TODO: Error should say it's ambiguous
}
// ----
// TypeError 2144: (134-147): No matching declaration found after variable lookup.
