contract C {
    string s;
    function f() public { uint a = s.length; }
}
// ----
// TypeError: (62-70): Member "length" not found or not visible after argument-dependent lookup in string storage ref.
