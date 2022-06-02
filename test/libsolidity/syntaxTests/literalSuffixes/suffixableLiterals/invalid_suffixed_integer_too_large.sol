function suffix(uint x) pure returns (uint) { return x; }

contract C {
    uint x = 10e1000 suffix;
    uint y = 999999999999999999999999999999999999999999999999999999999999999999999999999999 suffix;
}
// ----
// TypeError 8838: (85-99): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (114-199): The type of the literal cannot be converted to the parameter of the suffix function.
