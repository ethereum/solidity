function suffix(string memory s) pure returns (string memory) { return s; }

contract C {
    // TODO: This should be an error
    string s = "abcd" "" suffix;
}
