uint256 constant TWO = 2;
uint8 constant U8_255 = 255;
uint8 constant U8_240 = 240; // 1111 0000

contract C {
    uint256 constant LITERAL = 1 >> TWO;
    uint[LITERAL + 1] a;
    uint8 constant CONST = U8_240 >> 4;
    uint[CONST] b;
    uint constant CONST_BITS_DISCARDED = U8_255 >> 8;
    uint[CONST_BITS_DISCARDED + 1] c;
    uint8 constant CONST_ALL_DISCARDED = U8_240 >> 32;
    uint[CONST_ALL_DISCARDED + 1] d;

    function testLiteralEquivalence() public view returns (bool) {
        uint256 runtimeResult = 1 >> TWO;
        return
            LITERAL == runtimeResult &&
            a.length == 1;
    }

    function testConstEquivalence() public view returns (bool) {
        uint8 runtimeResult = U8_240 >> 4;
        return
            CONST == runtimeResult &&
            b.length == runtimeResult;
    }

    function testConsBitsDiscardEquivalence() public view returns (bool) {
        uint runtimeResult = U8_255 >> 8;
        return
            CONST_BITS_DISCARDED == runtimeResult &&
            c.length == 1;
    }

    function testConsAllBitsDiscardEquivalence() public view returns (bool) {
        uint runtimeResult = U8_240 >> 32;
        return
            CONST_ALL_DISCARDED == runtimeResult &&
            d.length == 1;
    }
}
// ----
// testLiteralEquivalence() -> true
// testConstEquivalence() -> true
// testConsBitsDiscardEquivalence() -> true
// testConsAllBitsDiscardEquivalence() -> true
