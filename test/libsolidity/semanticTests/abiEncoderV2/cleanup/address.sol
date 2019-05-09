pragma experimental ABIEncoderV2;

contract C {
    function g(address x) external pure returns (uint256 r) {
        assembly { r := x }
    }
    function f(uint256 a) external view returns (uint256) {
        address x;
        assembly { x := a }
        return this.g(x);
    }
}
// ----
// f(uint256): 0 -> 0
// g(address): 0 -> 0 # test validation as well as sanity check #
// f(uint256): 1 -> 1
// g(address): 1 -> 1
// f(uint256): 2 -> 2
// g(address): 2 -> 2
// f(uint256): 0xabcdef0123456789abcdef0123456789abcdefff -> 0xabcdef0123456789abcdef0123456789abcdefff
// g(address): 0xabcdef0123456789abcdef0123456789abcdefff -> 0xabcdef0123456789abcdef0123456789abcdefff
// f(uint256): 0xffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffff
// g(address): 0xffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffff
// f(uint256): 0x010000000000000000000000000000000000000000 -> 0
// g(address): 0x010000000000000000000000000000000000000000 -> FAILURE
// f(uint256): 0x01abcdef0123456789abcdef0123456789abcdefff -> 0xabcdef0123456789abcdef0123456789abcdefff
// g(address): 0x01abcdef0123456789abcdef0123456789abcdefff -> FAILURE
// f(uint256): 0x01ffffffffffffffffffffffffffffffffffffffff -> 0xffffffffffffffffffffffffffffffffffffffff
// g(address): 0x01ffffffffffffffffffffffffffffffffffffffff -> FAILURE
// f(uint256): -1 -> 0xffffffffffffffffffffffffffffffffffffffff
// g(address): -1 -> FAILURE
