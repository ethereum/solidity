function suffix(string memory s) pure suffix returns (string memory) { return s; }

contract C {
    // TODO: This should be an error
    string s = "abcd" "" suffix;
}
