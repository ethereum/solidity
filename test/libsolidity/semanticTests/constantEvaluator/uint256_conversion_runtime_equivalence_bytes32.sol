bytes32 constant FULL_HEX = 0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef;
bytes32 constant FULL_STRING = "1234567890abcdef1234567890abcdef";
bytes32 constant PARTIAL_STRING = "1234abcd";
bytes32 constant PARTIAL_HEX_STR = hex"4d41";
bytes32 constant FULL_HEX_STR = hex"1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef";
bytes32 constant EMPTY = "";

contract C {
    int[uint(FULL_HEX)] fullHex;
    int[uint(FULL_STRING)] fullStr;
    int[uint(PARTIAL_STRING)] partialStr;
    int[uint(PARTIAL_HEX_STR)] partialHexStr;
    int[uint(FULL_HEX_STR)] fullHexStr;
    int[uint(EMPTY) + 1] empty;

    function testHexRuntimeEquivalence() public view returns (bool) {
        uint runtimeFull = uint(FULL_HEX);

        return fullHex.length == runtimeFull;
    }
    function testStringRuntimeEquivalence() public view returns (bool) {
        uint runtimeFull = uint(FULL_STRING);
        uint runtimePartial = uint(PARTIAL_STRING);

        return fullStr.length == runtimeFull && partialStr.length == runtimePartial;
    }
    function testHexStrRuntimeEquivalence() public view returns (bool) {
        uint runtimeFull = uint(FULL_HEX_STR);
        uint runtimePartial = uint(PARTIAL_HEX_STR);

        return fullHexStr.length == runtimeFull && partialHexStr.length == runtimePartial;
    }
    function testEmptyRuntimeEquivalence() public view returns (bool) {
        uint runtimeEmpty = uint(EMPTY);

        return empty.length == runtimeEmpty + 1;
    }

}
// ----
// testHexRuntimeEquivalence() -> true
// testStringRuntimeEquivalence() -> true
// testHexStrRuntimeEquivalence() -> true
// testEmptyRuntimeEquivalence() -> true
