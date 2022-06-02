library L {
    function suffix(uint x) external pure returns (uint) { return x; }
}

contract C {
    uint x = 1000 L.suffix;
}
// ----
// DeclarationError 7920: (117-125): Identifier not found or not unique.
