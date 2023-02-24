function signatureSuffix(string memory) pure suffix returns (string memory) { return "f()"; }

contract C {
    function test() public pure returns (bytes memory) {
        return abi.encodeWithSignature("abcd" signatureSuffix);
    }
}
// ----
// test() -> 0x20, 4, 0x26121ff000000000000000000000000000000000000000000000000000000000
