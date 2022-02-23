contract C {
    function f(uint x, string memory y) internal {}
    function f(bytes memory y, int x) internal {}

    function call() internal {
        f({x: 1, y: "abc"});
    }
}
// ----
// TypeError 4487: (155-156): No unique declaration found after argument-dependent lookup.
