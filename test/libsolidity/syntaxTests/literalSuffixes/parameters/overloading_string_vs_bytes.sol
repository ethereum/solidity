function suffix(string memory) pure suffix returns (string memory) {}
function suffix(bytes memory) pure suffix returns (bytes memory) {}

contract C {
    bytes a = hex"abcd" suffix;
}
// ----
// TypeError 2144: (176-182): No matching declaration found after variable lookup.
