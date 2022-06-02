function iuSuffix(uint8) pure returns (int) {}
function iuSuffix(int8) pure returns (int) {}

contract C {
    int a = 127 iuSuffix;
}
// ----
// DeclarationError 7920: (123-131): Identifier not found or not unique.
