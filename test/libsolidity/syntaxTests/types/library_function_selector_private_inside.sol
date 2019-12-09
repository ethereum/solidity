library L {
    function f(uint256) private {}
    function g(uint256) public returns (uint256) {
        return f.selector;
    }
}
// ----
// TypeError: (113-123): Member "selector" not found or not visible after argument-dependent lookup in function (uint256).
