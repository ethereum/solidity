function uSuffix(uint8) pure returns (int) {}
function uSuffix(uint16) pure returns (int) {}

contract C {
    int a = 127 uSuffix;
}
// ----
// DeclarationError 7920: (123-130): Identifier not found or not unique.
