contract Sample {
    struct s {
        uint16 x;
        uint16 y;
        string a;
        string b;
    }
    s[2] public p;

    constructor() public {
        s memory m;
        m.x = 0xbbbb;
        m.y = 0xcccc;
        m.a = "hello";
        m.b = "world";
        p[0] = m;
    }
}
// ----
// p(uint256): 0x0 -> 0xbbbb, 0xcccc, 0x80, 0xc0, 0x05, "hello", 0x05, "world"
