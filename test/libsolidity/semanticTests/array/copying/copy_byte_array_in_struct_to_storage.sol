pragma abicoder               v2;
struct S {
    uint16 x;
    bytes a;
    uint16 y;
    bytes b;
}
contract C {
    uint padding;
    S data;

    function f() public returns (bytes memory, bytes memory) {
        S memory x;
        x.x = 7;
        x.b = "1234567890123456789012345678901 1234567890123456789012345678901 123456789";
        x.a = "abcdef";
        x.y = 9;
        data = x;
        return (data.a, data.b);
    }
    function g() public returns (bytes memory, bytes memory) {
        S memory x;
        x.x = 7;
        x.b = "12345678923456789";
        x.a = "1234567890123456789012345678901 1234567890123456789012345678901 123456789";
        x.y = 9;
        data = x;
        return (data.a, data.b);
    }
    function h() public returns (bytes memory, bytes memory) {
        S memory x;
        data = x;
        return (data.a, data.b);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x40, 0x80, 6, 0x6162636465660000000000000000000000000000000000000000000000000000, 0x49, 0x3132333435363738393031323334353637383930313233343536373839303120, 0x3132333435363738393031323334353637383930313233343536373839303120, 0x3132333435363738390000000000000000000000000000000000000000000000
// gas irOptimized: 179952
// gas legacy: 180694
// gas legacyOptimized: 180088
// g() -> 0x40, 0xc0, 0x49, 0x3132333435363738393031323334353637383930313233343536373839303120, 0x3132333435363738393031323334353637383930313233343536373839303120, 0x3132333435363738390000000000000000000000000000000000000000000000, 0x11, 0x3132333435363738393233343536373839000000000000000000000000000000
// gas irOptimized: 107332
// gas legacy: 107895
// gas legacyOptimized: 107254
// h() -> 0x40, 0x60, 0x00, 0x00
// storageEmpty -> 1
