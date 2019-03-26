contract C {
    function f(uint x) internal { }
    function f(uint x, uint y) internal { }
    function f(uint x, uint y, uint z) internal { }
    function call() internal {
        f({x:1, y: 2, z: 3});
        f({y:2, v: 10, z: 3});
    }
}
// ----
// TypeError: (214-215): No matching declaration found after argument-dependent lookup.
