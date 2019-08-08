pragma experimental ABIEncoderV2;
contract C {
    function f(uint256[][2][] calldata x) external returns (uint256) {
        return 42;
    }
    function g(uint256[][2][] calldata x) external returns (uint256) {
        return this.f(x);
    }
}
// ----
// g(uint256[][2][]): 0x20, 0x01, 0x20, 0x40, 0x60, 0x00, 0x00 -> 42
// g(uint256[][2][]): 0x20, 0x01, 0x20, 0x00, 0x00 -> 42
// g(uint256[][2][]): 0x20, 0x01, 0x20, 0x00 -> FAILURE
