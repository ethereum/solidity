pragma abicoder v2;

contract c {
    struct S {
        uint16 a;
        uint16 b;
        uint16[3] c;
        uint16[] d;
    }
    S[] data;

    function test(S calldata c) public returns (uint16, uint16, uint16, uint16) {
        data.push(c);
        return (data[0].a, data[0].b, data[0].c[2], data[0].d[2]);
    }
}
// ====
// compileViaYul: also
// ----
// test((uint16,uint16,uint16[3],uint16[])): 0x20, 2, 3, 0, 0, 4, 0xC0, 4, 0, 0, 5, 0, 0 -> 2, 3, 4, 5
// gas irOptimized: 139100
// gas legacy: 144835
// gas legacyOptimized: 139171
