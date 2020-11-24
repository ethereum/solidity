pragma abicoder               v2;


contract C {
    function f1(bytes[1] calldata a)
        external
        returns (uint256, uint256, uint256, uint256)
    {
        return (a[0].length, uint8(a[0][0]), uint8(a[0][1]), uint8(a[0][2]));
    }

    function f2(bytes[1] calldata a, bytes[1] calldata b)
        external
        returns (uint256, uint256, uint256, uint256, uint256, uint256, uint256)
    {
        return (
            a[0].length,
            uint8(a[0][0]),
            uint8(a[0][1]),
            uint8(a[0][2]),
            b[0].length,
            uint8(b[0][0]),
            uint8(b[0][1])
        );
    }

    function g1(bytes[2] calldata a)
        external
        returns (
            uint256,
            uint256,
            uint256,
            uint256,
            uint256,
            uint256,
            uint256,
            uint256
        )
    {
        return (
            a[0].length,
            uint8(a[0][0]),
            uint8(a[0][1]),
            uint8(a[0][2]),
            a[1].length,
            uint8(a[1][0]),
            uint8(a[1][1]),
            uint8(a[1][2])
        );
    }

    function g2(bytes[] calldata a) external returns (uint256[8] memory) {
        return [
            a.length,
            a[0].length,
            uint8(a[0][0]),
            uint8(a[0][1]),
            a[1].length,
            uint8(a[1][0]),
            uint8(a[1][1]),
            uint8(a[1][2])
        ];
    }
}

// found expectation comments:
// same offset for both arrays @ ABI_CHECK(

// ====
// compileViaYul: false
// ----
// f1(bytes[1]): 0x20, 0x20, 0x3, hex"0102030000000000000000000000000000000000000000000000000000000000" -> 0x3, 0x1, 0x2, 0x3
// f2(bytes[1],bytes[1]): 0x40, 0xa0, 0x20, 0x3, hex"0102030000000000000000000000000000000000000000000000000000000000", 0x20, 0x2, hex"0102000000000000000000000000000000000000000000000000000000000000" -> 0x3, 0x1, 0x2, 0x3, 0x2, 0x1, 0x2
// g1(bytes[2]): 0x20, 0x40, 0x80, 0x3, hex"0102030000000000000000000000000000000000000000000000000000000000", 0x3, hex"0405060000000000000000000000000000000000000000000000000000000000" -> 0x3, 0x1, 0x2, 0x3, 0x3, 0x4, 0x5, 0x6
// g1(bytes[2]): 0x20, 0x40, 0x40, 0x3, hex"0102030000000000000000000000000000000000000000000000000000000000" -> 0x3, 0x1, 0x2, 0x3, 0x3, 0x1, 0x2, 0x3
// g2(bytes[]): 0x20, 0x2, 0x40, 0x80, 0x2, hex"0102000000000000000000000000000000000000000000000000000000000000", 0x3, hex"0405060000000000000000000000000000000000000000000000000000000000" -> 0x2, 0x2, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6
