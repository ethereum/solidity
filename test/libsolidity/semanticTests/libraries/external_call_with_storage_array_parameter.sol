library L {
    function f(uint256[2] storage _a) external returns (uint256) {
        return _a[0] * _a[1];
    }
}

contract C {
    uint256[2] x;

    function g(uint256 _value) external returns (uint256) {
        x[0] = x[1] = _value;
        return L.f(x);
    }
}
// ====
// compileViaYul: also
// ----
// library: L
// g(uint256): 4 -> 16
