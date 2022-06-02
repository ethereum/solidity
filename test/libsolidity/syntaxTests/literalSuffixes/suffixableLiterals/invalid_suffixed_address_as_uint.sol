function suffix(uint value) pure returns (uint) { return value; }

contract C {
    uint x = 0x1234567890123456789012345678901234567890 suffix;
}
// ----
// TypeError 8838: (93-142): The type of the literal cannot be converted to the parameter of the suffix function.
