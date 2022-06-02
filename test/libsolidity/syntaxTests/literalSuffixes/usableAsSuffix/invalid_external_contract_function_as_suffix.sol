contract C {
    uint a = 1000 this.suffix;

    function suffix(uint x) external pure returns (uint) { return x; }
}
// ----
// DeclarationError 7920: (31-42): Identifier not found or not unique.
