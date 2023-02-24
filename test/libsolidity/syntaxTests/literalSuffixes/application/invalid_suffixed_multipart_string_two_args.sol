function suffix(string memory s, string memory) pure suffix returns (string memory) { return s; }

contract C {
    string s = "abcd" "" suffix;
}
// ----
// TypeError 2505: (127-143): Functions that take 2 arguments can only be used as literal suffixes for rational numbers.
