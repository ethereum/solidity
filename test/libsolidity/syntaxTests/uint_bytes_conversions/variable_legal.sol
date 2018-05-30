contract C {
    bytes1 b1;
    bytes2 b2;
    uint8 i8 = 8;
    uint16 i16 = 16;
    int8 si8 = 8;
    int16 si16 = 16;

    function f() view public {
        // Legal, of same size
        bytes1(i8);
        bytes1(si8);
        uint8(b1);
        bytes2(i16);
        bytes2(si16);
        uint16(b2);
    }
}
// ----
