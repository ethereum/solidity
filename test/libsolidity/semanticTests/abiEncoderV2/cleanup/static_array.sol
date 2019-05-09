pragma experimental ABIEncoderV2;

contract C {
    function gggggggg(uint8[2] calldata s) external pure returns (bytes memory) {
        s[0]; // only this will validate.
        return msg.data;
    }
    function f(uint256 a, uint256 b) public returns (bytes memory) {
        uint8[2] memory m = [0,0];
        assembly {
            mstore(m, a)
            mstore(add(m, 0x20), b)
        }
        return this.gggggggg(m);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256,uint256): 1, 1 -> 0x20, 0x44, hex"78b86ac6", 1, 1, hex"00000000000000000000000000000000000000000000000000000000"
// gggggggg(uint8[2]): 1, 1 -> 0x20, 0x44, hex"78b86ac6", 1, 1, hex"00000000000000000000000000000000000000000000000000000000"
// f(uint256,uint256): 0x0101, 0x0101 -> 0x20, 0x44, hex"78b86ac6", 1, 1, hex"00000000000000000000000000000000000000000000000000000000"
// gggggggg(uint8[2]): 0x0101, 0x0101 -> FAILURE
