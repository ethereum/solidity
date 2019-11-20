library L {
    function f(uint256) external pure {}
    function g(uint256) external view {}
}
contract C {
    function f() public pure returns (bytes4, bytes4) {
        return (L.f.selector, L.g.selector);
    }
}
// ----
