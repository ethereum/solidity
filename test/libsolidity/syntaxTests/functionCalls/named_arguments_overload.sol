contract C {
    function f(uint x) internal { }
    function f(uint x, uint y) internal { }
    function f(uint x, uint y, uint z) internal { }
    function call() internal {
        f({x: 1});
        f({x: 1, y: 2});
        f({y: 2, x: 1});
        f({x: 1, y: 2, z: 3});
        f({z: 3, x: 1, y: 2});
        f({y: 2, z: 3, x: 1});
    }
}
// ----
