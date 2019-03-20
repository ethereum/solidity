contract C {
    function f(uint x) internal { }
    function f(uint x, uint y) internal { }
    function f(uint x, uint y, uint z) internal { }
    function call() internal {
        f(1, 2);
        f(1);

        f({x: 1, y: 2});
        f({y: 2});
    }
}
// ----
// TypeError: (241-242): No matching declaration found after argument-dependent lookup.
