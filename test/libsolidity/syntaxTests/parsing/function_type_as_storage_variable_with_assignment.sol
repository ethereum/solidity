contract test {
    function f(uint x, uint y) public returns (uint a) {}
    function (uint, uint) internal returns (uint) f1 = f;
}
// ----
// Warning: (31-37): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (39-45): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (63-69): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (20-73): Function state mutability can be restricted to pure
