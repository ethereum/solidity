contract C {
    uint8 public a;
    uint16 public b;
    uint128 public c;
    uint public d;
    bytes1 public e;
    bytes20 public f;
    bytes32 public g;
    bool public h;
    address public i;
    constructor() {
        a = 3;
        b = 4;
        c = 5;
        d = 6;
        e = bytes1(uint8(0x7f));
        f = bytes20(uint160(0x6465616462656566313564656164000000000010));
        g = bytes32(uint256(0x6465616462656566313564656164000000000000000000000000000000000010));
        h = true;
        i = address(type(uint160).max / 3);
    }
}
// ====
// compileToEwasm: also
// ----
// a() -> 3
// b() -> 4
// c() -> 5
// d() -> 6
// e() -> 0x7f00000000000000000000000000000000000000000000000000000000000000
// f() -> 0x6465616462656566313564656164000000000010000000000000000000000000
// g() -> 0x6465616462656566313564656164000000000000000000000000000000000010
// h() -> true
// i() -> 0x5555555555555555555555555555555555555555
