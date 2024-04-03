error E(uint a, uint b);
contract C {
    function f() public pure {
        revert E(2, 7);
    }
    function encode_f() public pure returns (bytes memory) {
        return abi.encodeError(E, (2, 7));
    }
}
// ----
// f() -> FAILURE, hex"85208890", 2, 7
// encode_f() -> 0x20, 0x44, hex"85208890", 2, 7, hex"00000000000000000000000000000000000000000000000000000000"
