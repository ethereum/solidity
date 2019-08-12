pragma experimental ABIEncoderV2;
contract C {
    function f(uint256[][2][] calldata x) external returns (uint256) {
        x[0]; // trigger bounds checks
        return 23;
    }
}
// ----
// f(uint256[][2][]): 0x20, 0x01, 0x20, 0x40, 0x60, 0x00, 0x00 -> 23 # this is the common encoding for x.length == 1 && x[0][0].length == 0 && x[0][1].length == 0 #
// f(uint256[][2][]): 0x20, 0x01, 0x20, 0x00, 0x00 -> 23 # exotic, but still valid encoding #
// f(uint256[][2][]): 0x20, 0x01, 0x20, 0x00 -> FAILURE # invalid (too short) encoding, but no failure due to this PR #
