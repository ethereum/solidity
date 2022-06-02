function suffix(string memory s, string memory) pure returns (string memory) { return s; }

contract C {
    string s = "abcd" "" suffix;
}
// ----
// TypeError 4778: (120-136): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
