contract C {
    function f() public view returns (bytes4) {
        return f.selector;
    }
}
// ----
// TypeError: (76-86): Member "selector" not found or not visible after argument-dependent lookup in function () view returns (bytes4)
