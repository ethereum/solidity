contract C {
    struct str {
        uint8 a;
        uint16 b;
        uint248 c;
    }
    str data;

    function test() public returns (uint256) {
        data.a = 2;
        if (data.a != 2) return 2;
        data.b = 0xabcd;
        if (data.b != 0xabcd) return 3;
        data.c = 0x1234567890;
        if (data.c != 0x1234567890) return 4;
        if (data.a != 2) return 5;
        data.a = 8;
        if (data.a != 8) return 6;
        if (data.b != 0xabcd) return 7;
        data.b = 0xdcab;
        if (data.b != 0xdcab) return 8;
        if (data.c != 0x1234567890) return 9;
        data.c = 0x9876543210;
        if (data.c != 0x9876543210) return 10;
        return 1;
    }
}

// ====
// compileToEwasm: also
// ----
// test() -> 1
