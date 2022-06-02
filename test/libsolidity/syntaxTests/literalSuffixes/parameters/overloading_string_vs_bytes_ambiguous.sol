function suffix(string memory) pure returns (uint) {}
function suffix(bytes memory) pure returns (uint) {}

contract C {
    uint a = "abcd" suffix;
}
// ----
// DeclarationError 7920: (141-147): Identifier not found or not unique.
