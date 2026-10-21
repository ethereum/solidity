uint256 constant U256_ZERO = 0;
uint256 constant U256_MAX = 2**256 - 1; // 1111 ...
uint8 constant U8_ZERO = 0;
uint8 constant U8_MAX = 255; // 1111 1111
uint8 constant U253 = 253; // 1111 1101

contract C {
    uint256 constant NOT_U256_MAX = ~U256_MAX; // = 0
    uint256 constant NOT_U256_ZERO = ~U256_ZERO; // = 1111 ...
    uint8 constant NOT_U8_MAX = ~U8_MAX;
    uint8 constant NOT_U8_ZERO = ~U8_ZERO;
    uint8 constant UNSIGNED = ~U253; // = 2 (0000 0010)

    function testU256BoundariesEquivalence() public view returns (bool) {
        uint256[NOT_U256_MAX + 1] memory a;
        uint256[NOT_U256_ZERO - (2**256 - 2)] memory b;
        uint256 notMaxRuntimeResult = ~U256_MAX;
        uint256 notZeroRuntimeResult = ~U256_ZERO;

        return
            NOT_U256_MAX == notMaxRuntimeResult &&
            a.length == 1 &&
            NOT_U256_ZERO == notZeroRuntimeResult &&
            b.length == 1;
    }
    function testU8BoundariesEquivalence() public view returns (bool) {
        uint256[NOT_U8_MAX + 1] memory a;
        uint256[NOT_U8_ZERO] memory b;
        uint256 notMaxRuntimeResult = ~U8_MAX;
        uint256 notZeroRuntimeResult = ~U8_ZERO;

        return
            NOT_U8_MAX == notMaxRuntimeResult &&
            a.length == 1 &&
            NOT_U8_ZERO == notZeroRuntimeResult &&
            b.length == 255;
    }
    function testUnsignedEquivalence() public view returns (bool) {
        uint[UNSIGNED] memory a;
        uint8 runTimeResult = ~U253;

        return
            UNSIGNED == runTimeResult &&
            a.length == 2;
    }
}
// ----
// testUnsignedEquivalence() -> true
// testU256BoundariesEquivalence() -> true
// testU8BoundariesEquivalence() -> true
