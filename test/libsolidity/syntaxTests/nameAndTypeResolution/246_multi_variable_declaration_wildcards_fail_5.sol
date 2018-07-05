contract C {
    function one() public returns (uint);
    function f() public { var (,) = one(); }
}
// ----
// SyntaxError: (81-96): The use of the "var" keyword is disallowed. The declaration part of the statement can be removed, since it is empty.
// TypeError: (81-96): Wildcard both at beginning and end of variable declaration list is only allowed if the number of components is equal.
