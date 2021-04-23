contract C {
    uint8 a;
    fixed64x2 b;
    fixed16x5 c;
    uint8 d;
    function set() public {
        a = 1;
        b = -104.5;
        c = -0.1234;
        d = 99;
    }
    function getLowLevel() public view returns (bytes32 r) {
        assembly { r := sload(0) }
    }
    function getIntermediate() public view returns (uint8, bytes8, bytes2, uint8) {
        return (a, bytes8(b), bytes2(c), d);
    }
    function get() public view returns (uint8, fixed64x2, fixed16x5, uint8) {
        return (a, b, c, d);
    }
}
// ====
// compileViaYul: also
// ----
// getLowLevel() -> 0x00
// getIntermediate() -> 0, 0x00, 0x00, 0
// get() -> 0, 0x00, 0x00, 0
// set() ->
// getLowLevel() -> 0x63cfccffffffffffffd72e01
// getIntermediate() -> 1, 0xffffffffffffd72e000000000000000000000000000000000000000000000000, 0xcfcc000000000000000000000000000000000000000000000000000000000000, 0x63
// get() -> 1, -104.50, -0.12340, 0x63
