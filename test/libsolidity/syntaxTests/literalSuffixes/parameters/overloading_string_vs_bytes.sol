function suffix(string memory) pure returns (int) {}
function suffix(bytes memory) pure returns (int) {}

contract C {
    int a = hex"abcd" suffix; // TODO: Should match only bytes
}
// ----
// TypeError 2144: (131-147): No matching declaration found after variable lookup.
