function suffix(string memory value) pure returns (string memory) { return value; }

contract C {
    string x = hex"abcd" suffix;
}
// ----
// TypeError 8838: (113-129): The type of the literal cannot be converted to the parameter of the suffix function.
