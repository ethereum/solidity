function suffix(string memory) pure returns (uint) {}
function suffix(bytes memory) pure returns (uint) {}

contract C {
    uint a = "abcd" suffix; // TODO: Error should say it's ambiguous
}
// ----
// TypeError 2144: (134-147): No matching declaration found after variable lookup.
