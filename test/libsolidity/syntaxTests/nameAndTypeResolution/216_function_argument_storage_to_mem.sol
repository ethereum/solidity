contract C {
    function f(uint[] storage x) private {
        g(x);
    }
    function g(uint[] memory x) public {
    }
}
// ----
