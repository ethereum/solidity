function suffix(string memory s, string memory) pure suffix returns (string memory) { return s; }

contract C {
    string s = "abcd" "" suffix;
}
// ----
// TypeError 1587: (15-47): Literal suffix function has invalid parameter types. The mantissa parameter must be an integer. The exponent parameter must be an unsigned integer.
// TypeError 2505: (137-143): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
