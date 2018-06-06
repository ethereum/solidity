contract C {
    function one() public returns (uint);
    function f() public { var (,) = one(); }
}
// ----
// TypeError: (81-96): Wildcard both at beginning and end of variable declaration list is only allowed if the number of components is equal.
