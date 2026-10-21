uint256 constant ONE = 1;
int8 constant I8_NEGATIVE_63 = -63;
int8 constant I8_POSITIVE_127 = 127;
int16 constant I16_POSITIVE_127 = 127;

contract C {
    // right side cannot be signed
    int256 constant LITERAL_WRAP = -2**255 << ONE; // = 0
    uint[LITERAL_WRAP + 1] a;
    int8 constant CONST_NO_WRAP = I8_NEGATIVE_63 << 1;
    uint[CONST_NO_WRAP * -1] b;
    int8 constant CONST_WRAP = I8_POSITIVE_127 << 1; // = -2 (1111 1110)
    uint[CONST_WRAP * -1] c;
    int16 constant CONST_SIGN_CHANGED = I16_POSITIVE_127 << 9; // = -512 (1111 1110 0000 0000)
    uint[CONST_SIGN_CHANGED * -1] d;
    int8 constant CONST_NEGATIVE_I8_EXCEEDS_WIDTH = I8_NEGATIVE_63 << 32;
    uint[CONST_NEGATIVE_I8_EXCEEDS_WIDTH + 1] e;
    int256 constant CONST_POSITIVE_I8_EXCEEDS_WIDTH = I8_POSITIVE_127 << 32;
    uint[CONST_POSITIVE_I8_EXCEEDS_WIDTH + 1] f;

    function testLiteralWrapEquivalence() public view returns (bool) {
        int256 runTimeResult = -2**255 << ONE;

        return
            LITERAL_WRAP == runTimeResult &&
            a.length == 1;
    }

    function testConstNoWrapEquivalence() public view returns (bool) {
        int8 runTimeResult = I8_NEGATIVE_63 << 1;

        return
            CONST_NO_WRAP == runTimeResult &&
            b.length == 126;
    }

    function testConstWrapEquivalence() public view returns (bool) {
        int8 runTimeResult = I8_POSITIVE_127 << 1;

        return
            CONST_WRAP == runTimeResult &&
            c.length == 2;
    }

    function testConstSignChanged() public view returns (bool) {
        int16 runTimeResult = I16_POSITIVE_127 << 9;

        return
            CONST_SIGN_CHANGED == runTimeResult &&
            d.length == 512;
    }

    function testConstExceedsWidthEquivalence() public view returns (bool) {
        int8 runtimeResultNegative = I8_NEGATIVE_63 << 128;
        int8 runtimeResultPositive = I8_POSITIVE_127 << 128;

        return
            CONST_NEGATIVE_I8_EXCEEDS_WIDTH == runtimeResultNegative &&
            CONST_POSITIVE_I8_EXCEEDS_WIDTH == runtimeResultPositive &&
            e.length == 1 &&
            f.length == 1;
    }
}
// ----
// testLiteralWrapEquivalence() -> true
// testConstNoWrapEquivalence() -> true
// testConstWrapEquivalence() -> true
// testConstSignChanged() -> true
// testConstExceedsWidthEquivalence() -> true
