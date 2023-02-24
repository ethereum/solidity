function selectorSuffix(uint) pure suffix returns (bytes4) { return 0x12345678; }

contract C {
    function test() public pure returns (bytes memory) {
        return abi.encodeWithSelector(1234 selectorSuffix);
    }
}
// ----
// test() -> 0x20, 4, 0x1234567800000000000000000000000000000000000000000000000000000000
