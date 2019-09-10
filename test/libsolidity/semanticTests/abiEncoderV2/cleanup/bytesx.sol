pragma experimental ABIEncoderV2;

contract C {
    function gg1(bytes1 x) external pure returns (bytes32) {
        return x;
    }
    function f1(bytes32 a) external view returns (bytes32) {
        bytes1 x;
        assembly { x := a }
        return this.gg1(x);
    }
    function gg2(bytes2 x) external pure returns (bytes32) {
        return x;
    }
    function f2(bytes32 a) external view returns (bytes32) {
        bytes2 x;
        assembly { x := a }
        return this.gg2(x);
    }
    function gg4(bytes4 x) external pure returns (bytes32) {
        return x;
    }
    function f4(bytes32 a) external view returns (bytes32) {
        bytes4 x;
        assembly { x := a }
        return this.gg4(x);
    }
    function gg8(bytes8 x) external pure returns (bytes32) {
        return x;
    }
    function f8(bytes32 a) external view returns (bytes32) {
        bytes8 x;
        assembly { x := a }
        return this.gg8(x);
    }
    function g16(bytes16 x) external pure returns (bytes32) {
        return x;
    }
    function f16(bytes32 a) external view returns (bytes32) {
        bytes16 x;
        assembly { x := a }
        return this.g16(x);
    }
}
// ----
// f1(bytes32): left(0) -> left(0)
// gg1(bytes1): left(0) -> left(0) # test validation as well as sanity check #
// f1(bytes32): left(1) -> left(1)
// gg1(bytes1): left(1) -> left(1)
// f1(bytes32): left(0xFE) -> left(0xFE)
// gg1(bytes1): left(0xFE) -> left(0xFE)
// f1(bytes32): left(0xFF) -> left(0xFF)
// gg1(bytes1): left(0xFF) -> left(0xFF)
// f1(bytes32): left(0x0001) -> left(0x00)
// gg1(bytes1): left(0x0001) -> FAILURE
// f1(bytes32): left(0x0101) -> left(0x01)
// gg1(bytes1): left(0x0101) -> FAILURE
// f1(bytes32): -1 -> left(0xFF)
// gg1(bytes1): -1 -> FAILURE
// f2(bytes32): left(0) -> left(0)
// gg2(bytes2): left(0) -> left(0)
// f2(bytes32): left(1) -> left(1)
// gg2(bytes2): left(1) -> left(1)
// f2(bytes32): left(0xFFFE) -> left(0xFFFE)
// gg2(bytes2): left(0xFFFE) -> left(0xFFFE)
// f2(bytes32): left(0xFFFF) -> left(0xFFFF)
// gg2(bytes2): left(0xFFFF) -> left(0xFFFF)
// f2(bytes32): left(0x000001) -> left(0x00)
// gg2(bytes2): left(0x000001) -> FAILURE
// f2(bytes32): left(0x010001) -> left(0x01)
// gg2(bytes2): left(0x010001) -> FAILURE
// f2(bytes32): -1 -> left(0xFFFF)
// gg2(bytes2): -1 -> FAILURE
// f4(bytes32): left(0) -> left(0)
// gg4(bytes4): left(0) -> left(0)
// f4(bytes32): left(1) -> left(1)
// gg4(bytes4): left(1) -> left(1)
// f4(bytes32): left(0xFFFFFFFE) -> left(0xFFFFFFFE)
// gg4(bytes4): left(0xFFFFFFFE) -> left(0xFFFFFFFE)
// f4(bytes32): left(0xFFFFFFFF) -> left(0xFFFFFFFF)
// gg4(bytes4): left(0xFFFFFFFF) -> left(0xFFFFFFFF)
// f4(bytes32): left(0x0000000001) -> left(0x00)
// gg4(bytes4): left(0x0000000001) -> FAILURE
// f4(bytes32): left(0x0100000001) -> left(0x01)
// gg4(bytes4): left(0x0100000001) -> FAILURE
// f4(bytes32): -1 -> left(0xFFFFFFFF)
// gg4(bytes4): -1 -> FAILURE
// f8(bytes32): left(0) -> left(0)
// gg8(bytes8): left(0) -> left(0)
// f8(bytes32): left(1) -> left(1)
// gg8(bytes8): left(1) -> left(1)
// f8(bytes32): left(0xFFFFFFFFFFFFFFFE) -> left(0xFFFFFFFFFFFFFFFE)
// gg8(bytes8): left(0xFFFFFFFFFFFFFFFE) -> left(0xFFFFFFFFFFFFFFFE)
// f8(bytes32): left(0xFFFFFFFFFFFFFFFF) -> left(0xFFFFFFFFFFFFFFFF)
// gg8(bytes8): left(0xFFFFFFFFFFFFFFFF) -> left(0xFFFFFFFFFFFFFFFF)
// f8(bytes32): left(0x000000000000000001) -> left(0x00)
// gg8(bytes8): left(0x000000000000000001) -> FAILURE
// f8(bytes32): left(0x010000000000000001) -> left(0x01)
// gg8(bytes8): left(0x010000000000000001) -> FAILURE
// f8(bytes32): -1 -> left(0xFFFFFFFFFFFFFFFF)
// gg8(bytes8): -1 -> FAILURE
// f16(bytes32): left(0) -> left(0)
// g16(bytes16): left(0) -> left(0)
// f16(bytes32): left(1) -> left(1)
// g16(bytes16): left(1) -> left(1)
// f16(bytes32): left(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE) -> left(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE)
// g16(bytes16): left(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE) -> left(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE)
// f16(bytes32): left(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF) -> left(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
// g16(bytes16): left(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF) -> left(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
// f16(bytes32): left(0x0000000000000000000000000000000001) -> left(0x00)
// g16(bytes16): left(0x0000000000000000000000000000000001) -> FAILURE
// f16(bytes32): left(0x0100000000000000000000000000000001) -> left(0x01)
// g16(bytes16): left(0x0100000000000000000000000000000001) -> FAILURE
// f16(bytes32): -1 -> left(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
// g16(bytes16): -1 -> FAILURE
