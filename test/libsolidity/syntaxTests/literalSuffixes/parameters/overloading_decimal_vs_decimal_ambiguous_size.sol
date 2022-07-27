function uSuffix(uint8, uint) pure returns (uint) {}
function uSuffix(uint16, uint) pure returns (uint) {}

contract C {
    uint a = 1.27 uSuffix; // TODO: Error should say it's ambiguous
}
// ----
// TypeError 2144: (134-146): No matching declaration found after variable lookup.
