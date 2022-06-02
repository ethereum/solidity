function iuSuffix(uint8, uint) pure returns (uint) {}
function iuSuffix(int8, uint) pure returns (uint) {}

contract C {
    uint a = 1.27 iuSuffix;
}
// ----
// DeclarationError 7920: (139-147): Identifier not found or not unique.
