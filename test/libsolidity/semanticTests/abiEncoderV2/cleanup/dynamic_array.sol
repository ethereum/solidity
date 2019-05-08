pragma experimental ABIEncoderV2;

contract C {
    function ggg(uint8[] calldata s) external pure returns (bytes memory) {
        s[0]; // only this will validate.
        return msg.data;
    }
    function f(uint256[] calldata a) external returns (bytes memory) {
        uint8[] memory m = new uint8[](a.length);
        assembly {
            calldatacopy(add(m, 0x20), 0x44, mul(calldataload(4), 0x20))
        }
        return this.ggg(m);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256[]): 0x20, 2, 1, 1 -> 0x20, 0x84, hex"304a4c23", 0x20, 2, 1, 1, hex"00000000000000000000000000000000000000000000000000000000"
// ggg(uint8[]): 0x20, 2, 1, 1 -> 0x20, 0x84, hex"304a4c23", 0x20, 2, 1, 1, hex"00000000000000000000000000000000000000000000000000000000"
// f(uint256[]): 0x20, 2, 0x0101, 0x0101 -> 0x20, 0x84, hex"304a4c23", 0x20, 2, 1, 1, hex"00000000000000000000000000000000000000000000000000000000"
// ggg(uint8[]): 0x20, 2, 0x0101, 0x0101 -> FAILURE
