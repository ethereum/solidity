library L {
    function f(mapping(uint256 => uint256) storage _a) external returns (uint256) {
        return _a[0] * _a[1];
    }
}

contract C {
    mapping(uint256 => uint256) x;

    function g(uint256 _value) external returns (uint256) {
        x[0] = x[1] = _value;
        return L.f(x);
    }
}
// ----
// library: L
// g(uint256): 4 -> 16
