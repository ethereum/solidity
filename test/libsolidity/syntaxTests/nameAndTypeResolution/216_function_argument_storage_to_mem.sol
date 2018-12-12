contract C {
    function f(uint[] storage x) private {
        g(x);
    }
    function g(uint[] memory x) public {
    }
}
// ----
// Warning: (80-122): Function state mutability can be restricted to pure
