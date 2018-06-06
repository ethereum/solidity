contract C {
    string s;
    function f() public { var a = s[2]; }
}
// ----
// Warning: (53-58): Use of the "var" keyword is deprecated.
// TypeError: (61-65): Index access for string is not possible.
