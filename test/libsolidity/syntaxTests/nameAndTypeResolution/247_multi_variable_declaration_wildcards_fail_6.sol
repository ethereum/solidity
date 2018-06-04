contract C {
    function two() public returns (uint, uint);
    function f() public { var (a, b, c) = two(); }
}
// ----
// Warning: (92-93): Use of the "var" keyword is deprecated.
// Warning: (95-96): Use of the "var" keyword is deprecated.
// Warning: (98-99): Use of the "var" keyword is deprecated.
// Warning: (87-108): Different number of components on the left hand side (3) than on the right hand side (2).
// TypeError: (87-108): Not enough components (2) in value to assign all variables (3).
