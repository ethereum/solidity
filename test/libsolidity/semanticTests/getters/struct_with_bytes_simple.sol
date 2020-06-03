contract C {
    struct S {
        uint a;
        bytes b;
        mapping(uint => uint) c;
        uint[] d;
    }
    uint shifter;
    S public s;
    constructor() public {
        s.a = 7;
        s.b = "abc";
        s.c[0] = 9;
    }
}
// ----
// s() -> 0x07, 0x40, 0x03, 0x6162630000000000000000000000000000000000000000000000000000000000
