contract C {
    function g(int x, int y) public pure returns (int) { return x - y; }
    function h(int y, int x) public pure returns (int) { return y - x; }

    function f() public pure {
        (true ? g : h)({x : 1, y : 2});
        [g, h][1]({x : 1, y : 2});
    }
}
// ----
// TypeError 4974: (199-229): Named argument "x" does not match function declaration.
// TypeError 4974: (199-229): Named argument "y" does not match function declaration.
// TypeError 4974: (239-264): Named argument "x" does not match function declaration.
// TypeError 4974: (239-264): Named argument "y" does not match function declaration.
