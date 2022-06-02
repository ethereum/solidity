function suffix(uint x, uint y) pure returns (uint) { return x + y; }

contract C {
    using {suffix} for uint;

    uint a = 42;
    uint b = 1000 a.suffix;
}
// ----
// DeclarationError 7920: (149-157): Identifier not found or not unique.
