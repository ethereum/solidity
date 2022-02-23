contract C {
    function f(uint x, string memory y, bool z) internal {}
    function f(uint x, uint y, uint z) internal {}

    function call() internal {
        f({y: 1, x: "abc", z: true});
    }
}
// ----
// TypeError 9322: (164-165): No matching declaration found after argument-dependent lookup.
