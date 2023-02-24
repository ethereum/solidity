function suffix(string memory) pure suffix returns (uint) {}
function suffix(bytes memory) pure suffix returns (uint) {}

contract C {
    uint a = "abcd" suffix;
}
// ----
// TypeError 4487: (148-161): No unique declaration found after argument-dependent lookup.
