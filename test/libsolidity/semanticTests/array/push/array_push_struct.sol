contract c {
    struct S {
        uint16 a;
        uint16 b;
        uint16[3] c;
        uint16[] d;
    }
    S[] data;

    function test() public returns (uint16, uint16, uint16, uint16) {
        S memory s;
        s.a = 2;
        s.b = 3;
        s.c[2] = 4;
        s.d = new uint16[](4);
        s.d[2] = 5;
        data.push(s);
        return (data[0].a, data[0].b, data[0].c[2], data[0].d[2]);
    }
}
// ====
// compileViaYul: also
// ----
// test() -> 2, 3, 4, 5
// gas irOptimized: 136824
// gas legacy: 147484
// gas legacyOptimized: 146456
