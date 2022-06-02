function uSuffix(uint8, uint) pure returns (uint) {}
function uSuffix(uint16, uint) pure returns (uint) {}

contract C {
    uint a = 1.27 uSuffix;
}
// ----
// DeclarationError 7920: (139-146): Identifier not found or not unique.
