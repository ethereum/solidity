function suffix(string memory) pure suffix returns (uint) {}
function suffix(bytes memory) pure suffix returns (uint) {}

contract C {
    uint a = "abcd" suffix;
}
// ----
// TypeError 2144: (155-161): No matching declaration found after variable lookup.
