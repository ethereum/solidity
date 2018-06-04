contract C {
    function g() pure internal {
    }
    function f() view returns (bytes4) {
        return g.selector;
    }
}
// ----
// TypeError: (108-118): Member "selector" not found or not visible after argument-dependent lookup in function () pure
