error error(uint a);
contract C {
    function f() public pure {
        revert error(2);
    }
    function encode_f() public pure returns (bytes memory) {
        return abi.encodeError(error, (2));
    }
}
// ----
// f() -> FAILURE, hex"b48fb6cf", hex"0000000000000000000000000000000000000000000000000000000000000002"
// encode_f() -> 0x20, 0x24, hex"b48fb6cf", hex"0000000000000000000000000000000000000000000000000000000000000002", hex"00000000000000000000000000000000000000000000000000000000"
