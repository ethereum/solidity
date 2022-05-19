contract test {
    mapping(uint8 => uint8) a;
    mapping(uint8 => uint8) b;
    function f() internal returns (mapping(uint8 => uint8) storage r) {
        r = a;
        r[1] = 42;
        r = b;
        r[1] = 84;
    }
    function g() public returns (uint8, uint8, uint8, uint8, uint8, uint8) {
        f()[2] = 21;
        return (a[0], a[1], a[2], b[0], b[1], b[2]);
    }
    function h() public returns (uint8, uint8, uint8, uint8, uint8, uint8) {
        mapping(uint8 => uint8) storage m = f();
        m[2] = 17;
        return (a[0], a[1], a[2], b[0], b[1], b[2]);
    }
}
// ----
// g() -> 0, 42, 0, 0, 84, 21
// h() -> 0, 42, 0, 0, 84, 17
