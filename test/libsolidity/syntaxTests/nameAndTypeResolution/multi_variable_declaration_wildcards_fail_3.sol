contract C {
    function one() public returns (uint);
    function f() public { var (, , a) = one(); }
}
// ----
// Warning: (90-91): Use of the "var" keyword is deprecated.
// Warning: (81-100): Different number of components on the left hand side (3) than on the right hand side (1).
// TypeError: (81-100): Not enough components (1) in value to assign all variables (2).
