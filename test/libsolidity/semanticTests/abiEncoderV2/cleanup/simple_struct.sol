pragma experimental ABIEncoderV2;

contract C {
    struct S { uint8 a; bytes1 b; }
    function gg(S calldata s) external pure returns (bytes memory) {
        s.a; s.b; // only this will validate.
        return msg.data;
    }
    function f(uint256 a, bytes32 b) public returns (bytes memory) {
        S memory s = S(2,0x02);
        assembly {
            mstore(s, a)
            mstore(add(s, 0x20), b)
        }
        return this.gg(s);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256,bytes32): 1, left(0x01) -> 0x20, 0x44, hex"b63240b0", 1, left(0x01), hex"00000000000000000000000000000000000000000000000000000000"
// gg((uint8,bytes1)): 1, left(0x01) -> 0x20, 0x44, hex"b63240b0", 1, left(0x01), hex"00000000000000000000000000000000000000000000000000000000"
// f(uint256,bytes32): 0x0101, left(0x0101) -> 0x20, 0x44, hex"b63240b0", 1, left(0x01), hex"00000000000000000000000000000000000000000000000000000000"
// gg((uint8,bytes1)): 0x0101, left(0x0101) -> FAILURE
