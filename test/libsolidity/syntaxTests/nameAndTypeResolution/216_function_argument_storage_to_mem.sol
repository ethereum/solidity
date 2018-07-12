contract C {
    function f(uint[] storage x) private {
        g(x);
    }
    function g(uint[] memory x) public {
    }
}
// ----
// Warning: (91-106): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (80-122): Function state mutability can be restricted to pure
