library Lib {
    function f(mapping(uint => uint) storage a, mapping(uint => uint) storage b) internal returns(mapping(uint=>uint) storage r)
    {
        r = a;
        r[1] = 42;
        r = b;
        r[1] = 21;
    }
}
contract Test {
    mapping(uint => uint) a;
    mapping(uint => uint) b;
    function f() public returns (uint, uint, uint, uint, uint, uint)
    {
        Lib.f(a, b)[2] = 84;
        return (a[0], a[1], a[2], b[0], b[1], b[2]);
    }
    function g() public returns (uint, uint, uint, uint, uint, uint)
    {
        mapping(uint => uint) storage m = Lib.f(a, b);
        m[2] = 17;
        return (a[0], a[1], a[2], b[0], b[1], b[2]);
    }
}
// ----
// library: Lib
// f() -> 0, 0x2a, 0, 0, 0x15, 0x54
// g() -> 0, 0x2a, 0, 0, 0x15, 0x11
