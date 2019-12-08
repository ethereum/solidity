library L {
    function f(uint256 v) external returns (uint256) { return v*v; }
}
contract C {
    function g(uint256 v) external returns (uint256) {
        return L.f(v);
    }
}
// ----
// library: L
// g(uint256): 1 -> 1
// g(uint256): 2 -> 4
// g(uint256): 4 -> 16
