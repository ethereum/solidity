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
    function get() public view returns (uint8, bytes8, bytes2, uint8) {
        return (a, bytes8(b), bytes2(c), d);
    }
}
// ----
// z() ->
// f() ->
