uint256 constant U_256 = 256;
int8 constant I8_NEGATIVE_63 = -63;
int8 constant I8_POSITIVE_127 = 127;
int8 constant I8_NEGATIVE_128 = -128;

contract C {
    // right side cannot be signed
    int256 constant LITERAL = -2**255 >> U_256; // = -1
    uint[LITERAL * -1] a;
    int8 constant CONST = I8_NEGATIVE_63 >> 1; // = -32
    uint[CONST * -1] b;
    int8 constant CONST_BITS_DISCARDED = I8_POSITIVE_127 >> 8; // = 0
    uint[CONST_BITS_DISCARDED + 1] c;
    int8 constant CONST_SIGN_EXT = I8_NEGATIVE_128 >> 8; // = -1
    uint[CONST_SIGN_EXT * -1] d;
    int8 constant CONST_NEGATIVE_ALL_DISCARDED = I8_NEGATIVE_128 >> 32;
    uint[CONST_NEGATIVE_ALL_DISCARDED * -1] e;
    int8 constant CONST_POSITIVE_ALL_DISCARDED = I8_POSITIVE_127 >> 32;
    uint[CONST_POSITIVE_ALL_DISCARDED + 1] f;

    function testLiteralWrapEquivalence() public view returns (bool) {
        int256 runtimeResult = -2**255 >> U_256;

        return
            LITERAL == runtimeResult &&
            a.length == 1;
    }

    function testConstNoWrapEquivalence() public view returns (bool) {
        int8 runtimeResult = I8_NEGATIVE_63 >> 1;

        return
            CONST == runtimeResult &&
            b.length == 32;
    }

    function testConstWrapEquivalence() public view returns (bool) {
        int8 runtimeResult = I8_POSITIVE_127 >> 8;

        return
            CONST_BITS_DISCARDED == runtimeResult &&
            c.length == 1;
    }

    function testConstWrapSignExtendEquivalence() public view returns (bool) {
        int8 runtimeResult = I8_NEGATIVE_128 >> 8;

        return
            CONST_SIGN_EXT == runtimeResult &&
            d.length == 1;
    }

    function testConstPositiveAllBitsDiscarded() public view returns (bool) {
        int8 runtimeResultNegative = I8_NEGATIVE_128 >> 32;
        int8 runtimeResultPositive = I8_POSITIVE_127 >> 32;

        return
            CONST_NEGATIVE_ALL_DISCARDED == runtimeResultNegative &&
            CONST_POSITIVE_ALL_DISCARDED == runtimeResultPositive &&
            e.length == 1 &&
            f.length == 1;
    }
}
// ----
// testLiteralWrapEquivalence() -> true
// testConstNoWrapEquivalence() -> true
// testConstWrapEquivalence() -> true
// testConstWrapSignExtendEquivalence() -> true
// testConstPositiveAllBitsDiscarded() -> true
