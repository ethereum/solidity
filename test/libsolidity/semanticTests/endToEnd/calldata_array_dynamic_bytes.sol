pragma experimental ABIEncoderV2;
contract C {
    function f1(bytes[1] calldata a) external returns(uint256, uint256, uint256, uint256) {
        return (a[0].length, uint8(a[0][0]), uint8(a[0][1]), uint8(a[0][2]));
    }

    function f2(bytes[1] calldata a, bytes[1] calldata b) external returns(uint256, uint256, uint256, uint256, uint256, uint256, uint256) {
        return (a[0].length, uint8(a[0][0]), uint8(a[0][1]), uint8(a[0][2]), b[0].length, uint8(b[0][0]), uint8(b[0][1]));
    }

    function g1(bytes[2] calldata a) external returns(uint256, uint256, uint256, uint256, uint256, uint256, uint256, uint256) {
        return (a[0].length, uint8(a[0][0]), uint8(a[0][1]), uint8(a[0][2]), a[1].length, uint8(a[1][0]), uint8(a[1][1]), uint8(a[1][2]));
    }

    function g2(bytes[] calldata a) external returns(uint256[8] memory) {
        return [a.length, a[0].length, uint8(a[0][0]), uint8(a[0][1]), a[1].length, uint8(a[1][0]), uint8(a[1][1]), uint8(a[1][2])];
    }
}

// ----
// f1(bytes[1]): 0x20, 0x20, 3, 1, 2, 3, 29, 0 -> 3, 0, 0, 0
// f2(bytes[1],bytes[1]): 0x40, 0xA0, 0x20, 3, 1, 2, 3, 29, 0, 0x20, 2, 1, 2, 30, 0 -> FAILURE
// g1(bytes[2]): 0x20, 0x40, 0x80, 3, 1, 2, 3, 29, 0, 3, 4, 5, 6, 29, 0 -> FAILURE
// g1(bytes[2]): 0x20, 0x40, 0x40, 3, 1, 2, 3, 29, 0 -> 3, 0, 0, 0, 3, 0, 0, 0
// g2(bytes[]): 0x20, 2, 0x40, 0x80, 2, 1, 2, 30, 0, 3, 4, 5, 6, 29, 0 -> FAILURE
