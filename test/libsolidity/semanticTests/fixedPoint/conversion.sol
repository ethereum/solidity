contract C {
    function test() public pure returns (uint a, uint b, uint c, uint d) {
        fixed64x4 x = -1.1234;
        a = uint64(bytes8(x));
        fixed64x2 y = fixed64x2(x);
        b = uint64(bytes8(y));
        fixed16x4 z = fixed16x4(x);
        c = uint16(bytes2(z));
        ufixed64x4 w = ufixed64x4(x);
        d = uint64(bytes8(w));
    }
}
// ---
// test() ->
