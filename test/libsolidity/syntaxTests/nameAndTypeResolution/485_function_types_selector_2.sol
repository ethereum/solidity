contract C {
    function g() pure internal {
    }
    function f() public view returns (bytes4) {
        return g.selector;
    }
}
// ----
// TypeError 9582: (115-125='g.selector'): Member "selector" not found or not visible after argument-dependent lookup in function () pure.
