contract C {
    function one() public returns (uint);
    function f() public { (uint a, uint b, ) = one(); }
}
// ----
// Warning: (81-107): Different number of components on the left hand side (3) than on the right hand side (1).
// TypeError: (81-107): Not enough components (1) in value to assign all variables (2).
