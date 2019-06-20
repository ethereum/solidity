pragma experimental ABIEncoderV2;

// tests encoding from memory arrays

contract C {
    function f(uint256[][] calldata s) external returns (bytes memory) {
        uint256[][] memory tmp_f = s;
        return abi.encode(tmp_f);
    }
    function g(uint256[][2] calldata s) external returns (bytes memory) {
        uint256[][2] memory tmp_g = s;
        return abi.encode(tmp_g);
    }
    function h(uint256[2][] calldata s) external returns (bytes memory) {
        uint256[2][] memory tmp_h = s;
        return abi.encode(tmp_h);
    }
    function i(uint256[2][2] calldata s) external returns (bytes memory) {
        uint256[2][2] memory tmp_i = s;
        return abi.encode(tmp_i);
    }
    function j(bytes[2] calldata s) external returns (bytes memory) {
        bytes[2] memory tmp_j = s;
        return abi.encode(tmp_j);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256[][])  : 0x20, 2, 0x40, 0xC0, 3, 13, 17, 23, 4, 27, 31, 37, 41 -> 32, 416, 32, 2, 64, 192, 3, 13, 17, 23, 4, 27, 31, 37, 41
// g(uint256[][2]) : 0x20, 0x40, 0xC0, 3, 123, 124, 125, 3, 223, 224, 225 -> 32, 352, 0x20, 0x40, 0xC0, 3, 123, 124, 125, 3, 223, 224, 225
// h(uint256[2][]) : 0x20, 3, 123, 124, 223, 224, 323, 324 -> 32, 256, 0x20, 3, 123, 124, 223, 224, 323, 324
// i(uint256[2][2]): 123, 124, 223, 224 -> 32, 128, 123, 124, 223, 224
// j(bytes[2])     : 0x20, 0x40, 0x66, 6, hex"123ABC456DEF", 4, hex"FED65432" -> 32, 224, 0x20, 0x40, 0x80, 6, left(0x123ABC456DEF), 4, left(0xFED65432)
