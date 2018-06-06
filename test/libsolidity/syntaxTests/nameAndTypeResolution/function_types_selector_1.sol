contract C {
    function f() view returns (bytes4) {
        return f.selector;
    }
}
// ----
// TypeError: (69-79): Member "selector" not found or not visible after argument-dependent lookup in function () view returns (bytes4)
