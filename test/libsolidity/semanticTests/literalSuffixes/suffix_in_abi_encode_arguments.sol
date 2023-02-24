function uintSuffix(uint x) pure suffix returns (uint) { return x; }

contract C {
    function g(uint) public {}

    function test() public pure returns (bytes memory) {
        return abi.encode(1 uintSuffix);
    }

    function testPacked() public pure returns (bytes memory) {
        return abi.encodePacked(2 uintSuffix);
    }

    function testWithSelector() public pure returns (bytes memory) {
        return abi.encodeWithSelector(0x12345678, 3 uintSuffix);
    }

    function testWithSignature() public pure returns (bytes memory) {
        return abi.encodeWithSignature("f()", 4 uintSuffix);
    }

    function testCall() public view returns (bytes memory) {
        return abi.encodeCall(this.g, 5 uintSuffix);
    }
}
// ----
// test() -> 0x20, 0x20, 1
// testPacked() -> 0x20, 0x20, 2
// testWithSelector() -> 0x20, 0x24, 0x1234567800000000000000000000000000000000000000000000000000000000, 0x0000000300000000000000000000000000000000000000000000000000000000
// testWithSignature() -> 0x20, 0x24, 0x26121ff000000000000000000000000000000000000000000000000000000000, 0x0000000400000000000000000000000000000000000000000000000000000000
// testCall() -> 0x20, 0x24, 0xe420264a00000000000000000000000000000000000000000000000000000000, 0x0000000500000000000000000000000000000000000000000000000000000000
