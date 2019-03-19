contract C {
    function f(uint x) internal { }
    function f(uint x, uint y) internal { }
    function f(uint x, uint y, uint z) internal { }
    function call() internal {

        f({x:1, y: 2});
        f({x:1, z: 3});
    }
}
// ----
// TypeError: (209-210): No matching declaration found after argument-dependent lookup.
