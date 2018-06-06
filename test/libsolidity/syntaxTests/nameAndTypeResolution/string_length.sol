contract C {
    string s;
    function f() public { var a = s.length; }
}
// ----
// Warning: (53-58): Use of the "var" keyword is deprecated.
// TypeError: (61-69): Member "length" not found or not visible after argument-dependent lookup in string storage ref
