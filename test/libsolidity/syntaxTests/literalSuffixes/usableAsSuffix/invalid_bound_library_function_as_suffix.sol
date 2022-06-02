library L {
    function suffix(uint x, uint y) internal pure returns (uint) { return x + y; }
}

contract C {
    using L for uint;

    uint a = 42;
    uint b = 1000 a.suffix;
}
// ----
// DeclarationError 7920: (169-177): Identifier not found or not unique.
