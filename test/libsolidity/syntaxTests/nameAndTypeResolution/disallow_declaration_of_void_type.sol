contract c {
    function f() public { var (x) = f(); }
}
// ----
// Warning: (44-45): Use of the "var" keyword is deprecated.
// Warning: (39-52): Different number of components on the left hand side (1) than on the right hand side (0).
// TypeError: (39-52): Not enough components (0) in value to assign all variables (1).
