function suffix(string memory value) pure suffix returns (string memory) { return value; }

contract C {
    string x = hex"abcd" suffix;
}
// ----
// TypeError 8838: (120-129): The literal cannot be converted to type string memory accepted by the suffix function.
