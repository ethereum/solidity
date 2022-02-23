contract C {
    string s;
    function f() public { bytes1 a = s[2]; }
}
// ----
// TypeError 9961: (64-68): Index access for string is not possible.
