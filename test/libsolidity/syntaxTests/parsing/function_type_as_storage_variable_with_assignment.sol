contract test {
    function f(uint x, uint y) returns (uint a) {}
    function (uint, uint) internal returns (uint) f1 = f;
}
// ----
// Warning: (20-66): No visibility specified. Defaulting to "public". 
// Warning: (31-37): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (39-45): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (56-62): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (20-66): Function state mutability can be restricted to pure
