contract C {
    struct S {
        uint a;
        bytes b;
        mapping(uint => uint) c;
        uint[] d;
    }
    uint shifter;
    S public s;
    constructor() {
        s.a = 7;
        s.b = "abc";
        s.c[0] = 9;
        s.d.push(10);
    }
}
// ====
// compileViaYul: also
// ----
// s() -> 7, 0x40, 3, 0x6162630000000000000000000000000000000000000000000000000000000000
