function half(int value) pure suffix returns (int) { return value / 2; }
function half128(uint128 value) pure suffix returns (uint128) { return value / 2; }
function half64s(int64 value) pure suffix returns (int64) { return value / 2; }

contract C {
    function zero() public pure returns (int) {
        return 0 half + int128(0 half128) + 0 half64s;
    }

    function two() public pure returns (int) {
        return 2 half + int128(2 half128) + 2 half64s;
    }

    function max() public pure returns (int) {
        return
            0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff half +
            int128(0xffffffffffffffffffffffffffffffff half128) +
            0x7fffffffffffffff half64s;
    }

    function withDecimals() public pure returns (int) {
        return
            2.0 half +
            int128(0.0004e5 half128) +
            80000e-2 half64s;
    }
}
// ----
// zero() -> 0
// two() -> 3
// max() -> 0x4000000000000000000000000000000080000000000000003ffffffffffffffd
// withDecimals() -> 421
