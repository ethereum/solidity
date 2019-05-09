pragma experimental ABIEncoderV2;

contract C {
    function ggg8(int8 x) external pure returns (int256) {
        return x;
    }
    function gg16(int16 x) external pure returns (int256) {
        return x;
    }
    function gg32(int32 x) external pure returns (int256) {
        return x;
    }
    function gg64(int64 x) external pure returns (int256) {
        return x;
    }
    function g128(int128 x) external pure returns (int256) {
        return x;
    }
    function f8(int256 a) external view returns (int256) {
        int8 x = 0;
        assembly { x := a }
        return this.ggg8(x);
    }
    function f16(int256 a) external view returns (int256) {
        int16 x = 0;
        assembly { x := a }
        return this.gg16(x);
    }
    function f32(int256 a) external view returns (int256) {
        int32 x = 0;
        assembly { x := a }
        return this.gg32(x);
    }
    function f64(int256 a) external view returns (int256) {
        int64 x = 0;
        assembly { x := a }
        return this.gg64(x);
    }
    function f128(int256 a) external view returns (int256) {
        int128 x = 0;
        assembly { x := a }
        return this.g128(x);
    }
}
// ----
// f8(int256): 0 -> 0
// ggg8(int8): 0 -> 0 # test validation as well as sanity check #
// f8(int256): 1 -> 1
// ggg8(int8): 1 -> 1
// f8(int256): -1 -> -1
// ggg8(int8): -1 -> -1
// f8(int256): 0x7F -> 0x7F
// ggg8(int8): 0x7F -> 0x7F
// f8(int256): 0x80 -> -128
// ggg8(int8): 0x80 -> FAILURE
// f8(int256): 0xFE -> -2
// ggg8(int8): 0xFE -> FAILURE
// f8(int256): 0xFF -> -1
// ggg8(int8): 0xFF -> FAILURE
// f8(int256): 0x0100 -> 0x00
// ggg8(int8): 0x0100 -> FAILURE
// f8(int256): 0x0101 -> 0x01
// ggg8(int8): 0x0101 -> FAILURE
// f8(int256): 0x01FF -> -1
// ggg8(int8): 0x01FF -> FAILURE
// f8(int256): 0x01FE -> -2
// ggg8(int8): 0x01FE -> FAILURE
// f16(int256): 0 -> 0
// gg16(int16): 0 -> 0
// f16(int256): 1 -> 1
// gg16(int16): 1 -> 1
// f16(int256): -1 -> -1
// gg16(int16): -1 -> -1
// f16(int256): 0x7FFF -> 0x7FFF
// gg16(int16): 0x7FFF -> 0x7FFF
// f16(int256): 0x8000 -> -32768
// gg16(int16): 0x8000 -> FAILURE
// f16(int256): 0xFFFE -> -2
// gg16(int16): 0xFFFE -> FAILURE
// f16(int256): 0xFFFF -> -1
// gg16(int16): 0xFFFF -> FAILURE
// f16(int256): 0x010000 -> 0x00
// gg16(int16): 0x010000 -> FAILURE
// f16(int256): 0x010001 -> 0x01
// gg16(int16): 0x010001 -> FAILURE
// f16(int256): 0x01FFFF -> -1
// gg16(int16): 0x01FFFF -> FAILURE
// f16(int256): 0x01FFFE -> -2
// gg16(int16): 0x01FFFE -> FAILURE
// f32(int256): 0 -> 0
// gg32(int32): 0 -> 0
// f32(int256): 1 -> 1
// gg32(int32): 1 -> 1
// f32(int256): -1 -> -1
// gg32(int32): -1 -> -1
// f32(int256): 0x7FFFFFFF -> 0x7FFFFFFF
// gg32(int32): 0x7FFFFFFF -> 0x7FFFFFFF
// f32(int256): 0x80000000 -> -2147483648
// gg32(int32): 0x80000000 -> FAILURE
// f32(int256): 0xFFFFFFFE -> -2
// gg32(int32): 0xFFFFFFFE -> FAILURE
// f32(int256): 0xFFFFFFFF -> -1
// gg32(int32): 0xFFFFFFFF -> FAILURE
// f32(int256): 0x0100000000 -> 0x00
// gg32(int32): 0x0100000000 -> FAILURE
// f32(int256): 0x0100000001 -> 0x01
// gg32(int32): 0x0100000001 -> FAILURE
// f32(int256): 0x01FFFFFFFF -> -1
// gg32(int32): 0x01FFFFFFFF -> FAILURE
// f32(int256): 0x01FFFFFFFE -> -2
// gg32(int32): 0x01FFFFFFFE -> FAILURE
// f64(int256): 0 -> 0
// gg64(int64): 0 -> 0
// f64(int256): 1 -> 1
// gg64(int64): 1 -> 1
// f64(int256): -1 -> -1
// gg64(int64): -1 -> -1
// f64(int256): 0x7FFFFFFFFFFFFFFF -> 0x7FFFFFFFFFFFFFFF
// gg64(int64): 0x7FFFFFFFFFFFFFFF -> 0x7FFFFFFFFFFFFFFF
// f64(int256): 0x8000000000000000 -> -9223372036854775808
// gg64(int64): 0x8000000000000000 -> FAILURE
// f64(int256): 0xFFFFFFFFFFFFFFFE -> -2
// gg64(int64): 0xFFFFFFFFFFFFFFFE -> FAILURE
// f64(int256): 0xFFFFFFFFFFFFFFFF -> -1
// gg64(int64): 0xFFFFFFFFFFFFFFFF -> FAILURE
// f64(int256): 0x010000000000000000 -> 0x00
// gg64(int64): 0x010000000000000000 -> FAILURE
// f64(int256): 0x010000000000000001 -> 0x01
// gg64(int64): 0x010000000000000001 -> FAILURE
// f64(int256): 0x01FFFFFFFFFFFFFFFF -> -1
// gg64(int64): 0x01FFFFFFFFFFFFFFFF -> FAILURE
// f64(int256): 0x01FFFFFFFFFFFFFFFE -> -2
// gg64(int64): 0x01FFFFFFFFFFFFFFFE -> FAILURE
// f128(int256): 0 -> 0
// g128(int128): 0 -> 0
// f128(int256): 1 -> 1
// g128(int128): 1 -> 1
// f128(int256): -1 -> -1
// g128(int128): -1 -> -1
// f128(int256): 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF -> 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
// g128(int128): 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF -> 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
// f128(int256): 0x80000000000000000000000000000000 -> -170141183460469231731687303715884105728
// g128(int128): 0x80000000000000000000000000000000 -> FAILURE
// f128(int256): 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE -> -2
// g128(int128): 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE -> FAILURE
// f128(int256): 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF -> -1
// g128(int128): 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF -> FAILURE
// f128(int256): 0x0100000000000000000000000000000000 -> 0x00
// g128(int128): 0x0100000000000000000000000000000000 -> FAILURE
// f128(int256): 0x0100000000000000000000000000000001 -> 0x01
// g128(int128): 0x0100000000000000000000000000000001 -> FAILURE
// f128(int256): 0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF -> -1
// g128(int128): 0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF -> FAILURE
// f128(int256): 0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE -> -2
// g128(int128): 0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE -> FAILURE
