pragma experimental ABIEncoderV2;

contract C {
    function gggg(bool x) external pure returns (bool) {
        return x;
    }
    function f(uint256 a) external view returns (bool) {
        bool x = false;
        assembly { x := a }
        return this.gggg(x);
    }
}
// ----
// f(uint256): 0 -> false
// gggg(bool): 0 -> false # test validation as well as sanity check #
// f(uint256): 1 -> true
// gggg(bool): 1 -> true
// f(uint256): 2 -> true
// gggg(bool): 2 -> FAILURE
// f(uint256): 0x1000 -> true
// gggg(bool): 0x1000 -> FAILURE
