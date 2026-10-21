int8 constant I8_NEGATIVE = -2; // 1111 1110
int8 constant I8_POSITIVE = 1; // 0000 0001
int8 constant I8_MAX = 127; // 0111 1111
int8 constant I8_MIN = -128; // 1000 0000
int constant I256_MAX = 2**255 - 1; // 0111 1111 ...
int constant I256_MIN = -2**255; // 1000 0000 ...

contract C {
    int8 constant NOT_POSITIVE = ~I8_POSITIVE; // = -2 (1111 1110)
    int8 constant NOT_NEGATIVE = ~I8_NEGATIVE; // = 1
    int8 constant NOT_I8_MAX = ~I8_MAX; // = -128 (1000 0000)
    int8 constant NOT_I8_MIN = ~I8_MIN; // = 127 (0111 1111)
    int constant NOT_I256_MAX = ~I256_MAX; // 1000 0000 ...
    int constant NOT_I256_MIN = ~I256_MIN; // 0111 1111 ...

    function testNegativeSignedEquivalence() public view returns (bool) {
        uint[NOT_NEGATIVE] memory a;
        int8 runTimeResult = ~I8_NEGATIVE;

        return
            NOT_NEGATIVE == runTimeResult &&
            a.length == 1;
    }
    function testPositiveSignedEquivalence() public view returns (bool) {
        uint[NOT_POSITIVE * -1] memory a;
        int8 runTimeResult = ~I8_POSITIVE;

        return
            NOT_POSITIVE == runTimeResult &&
            a.length == 2;
    }
    function testI8BoundariesEquivalence() public view returns (bool) {
        uint[(NOT_I8_MAX / 2) * -1] memory a;
        uint[NOT_I8_MIN] memory b;
        int8 notMaxRuntimeResult = ~I8_MAX;
        int8 notMinRuntimeResult = ~I8_MIN;

        return
            NOT_I8_MAX == notMaxRuntimeResult &&
            a.length == 64 &&
            NOT_I8_MIN == notMinRuntimeResult &&
            b.length == 127;
    }
    function testI256BoundariesEquivalence() public view returns (bool) {
        uint[(NOT_I256_MAX + I256_MAX) * -1] memory a;
        uint[NOT_I256_MIN - I256_MAX + 1] memory b;
        int notMaxRuntimeResult = ~I256_MAX;
        int notMinRuntimeResult = ~I256_MIN;

        return
            NOT_I256_MAX == notMaxRuntimeResult &&
            a.length == 1 &&
            NOT_I256_MIN == notMinRuntimeResult &&
            b.length == 1;
    }
}
// ----
// testNegativeSignedEquivalence() -> true
// testPositiveSignedEquivalence() -> true
// testI8BoundariesEquivalence() -> true
// testI256BoundariesEquivalence() -> true
