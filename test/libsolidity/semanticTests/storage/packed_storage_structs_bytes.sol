contract C {
    struct s1 {
        bytes1 a;
        bytes1 b;
        bytes10 c;
        bytes9 d;
        bytes10 e;
    }
    struct s2 {
        bytes1 a;
        s1 inner;
        bytes1 b;
        bytes1 c;
    }
    bytes1 x;
    s2 data;
    bytes1 y;

    function test() public returns (bool) {
        x = 0x01;
        data.a = 0x02;
        data.inner.a = 0x03;
        data.inner.b = 0x04;
        data.inner.c = "1234567890";
        data.inner.d = "123456789";
        data.inner.e = "abcdefghij";
        data.b = 0x05;
        data.c = bytes1(0x06);
        y = 0x07;
        return
            x == 0x01 &&
            data.a == 0x02 &&
            data.inner.a == 0x03 &&
            data.inner.b == 0x04 &&
            data.inner.c == "1234567890" &&
            data.inner.d == "123456789" &&
            data.inner.e == "abcdefghij" &&
            data.b == 0x05 &&
            data.c == bytes1(0x06) &&
            y == 0x07;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> true
// gas irOptimized: 134587
// gas legacy: 136036
// gas legacyOptimized: 133480
