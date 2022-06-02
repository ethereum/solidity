function suffix(string memory) pure returns (int) {}
function suffix(bytes memory) pure returns (int) {}

contract C {
    int a = hex"abcd" suffix; // TODO: Should match only bytes
}
// ----
// DeclarationError 7920: (141-147): Identifier not found or not unique.
