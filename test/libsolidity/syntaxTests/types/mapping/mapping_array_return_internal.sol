contract C {
    mapping(uint=>uint)[] m;
    function f() internal view returns (mapping(uint=>uint)[] storage) {
        return m;
    }
    function g() private view returns (mapping(uint=>uint)[] storage) {
        return m;
    }
    function h() internal view returns (mapping(uint=>uint)[] storage r) {
        r = m;
    }
    function i() private view returns (mapping(uint=>uint)[] storage r) {
        (r,r) = (m,m);
    }
}
// ----
