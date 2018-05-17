contract test {
    function f(uint x, uint y) returns (uint a) {}
    function g() {
        function (uint, uint) internal returns (uint) f1 = f;
    }
}
// ----
// Warning: (20-66): No visibility specified. Defaulting to "public". 
// Warning: (31-37): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (39-45): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (56-62): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (71-153): No visibility specified. Defaulting to "public". 
// Warning: (94-142): Unused local variable.
// Warning: (20-66): Function state mutability can be restricted to pure
// Warning: (71-153): Function state mutability can be restricted to pure
