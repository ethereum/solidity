function suffix(string memory) pure returns (uint) {}
function suffix(bytes memory) pure returns (uint) {}

contract C {
    uint a = "abcd" suffix;
}
// ----
// TypeError 4487: (134-147): No unique declaration found after argument-dependent lookup.
