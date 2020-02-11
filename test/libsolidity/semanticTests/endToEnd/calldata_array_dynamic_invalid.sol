pragma experimental ABIEncoderV2;
contract C {
    function f(uint256[][] calldata a) external returns(uint) {
        return 42;
    }

    function g(uint256[][] calldata a) external returns(uint) {
        a[0];
        return 42;
    }
}

// ----
// f(uint256[][]): 0x20, 0 -> 42
// f(uint256[][]):"32, 0" -> "42"
// f(uint256[][]): 0x20, 1 -> 
// f(uint256[][]):"32, 1" -> ""
// f(uint256[][]): 0x20, 1, 0x20 -> 42
// f(uint256[][]):"32, 1, 32" -> "42"
// g(uint256[][]): 0x20, 1, 0x20 -> 
// g(uint256[][]):"32, 1, 32" -> ""
// f(uint256[][]): 0x20, 1, 0x20, 2, 0x42 -> 42
// f(uint256[][]):"32, 1, 32, 2, 66" -> "42"
// g(uint256[][]): 0x20, 1, 0x20, 2, 0x42 -> 
// g(uint256[][]):"32, 1, 32, 2, 66" -> ""
