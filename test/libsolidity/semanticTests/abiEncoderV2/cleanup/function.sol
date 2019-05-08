pragma experimental ABIEncoderV2;

contract C {
    struct S { function() external f; }
    function ggg(function() external x) external pure returns (uint256 r) {
        assembly { r := calldataload(4) }
    }
    function h(S calldata x) external pure returns (uint256 r) {
        x.f; // validation only happens here
        assembly { r := calldataload(4) }
    }
    function dummy() external {}
    function ffff(uint256 a) external view returns (uint256, uint256) {
        S memory s = S(this.dummy);
        assembly { mstore(s, a) }
        return (this.ggg(s.f), this.h(s));
    }
}
// ----
// ffff(uint256): 0 -> 0, 0
// ggg(function): 0 -> 0
// ffff(uint256): "01234567890123456789abcd" -> "01234567890123456789abcd", "01234567890123456789abcd"
// ggg(function): "01234567890123456789abcd" -> "01234567890123456789abcd"
// h((function)): "01234567890123456789abcd" -> "01234567890123456789abcd"
// h((function)): 0 -> 0
// ffff(uint256): "01234567890123456789abcdX" -> "01234567890123456789abcd", "01234567890123456789abcd"
// ggg(function): "01234567890123456789abcdX" -> FAILURE
// h((function)): "01234567890123456789abcdX" -> FAILURE
