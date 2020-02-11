pragma experimental ABIEncoderV2;
contract C {
    function f(uint256[][1][] calldata a) external returns(uint) {
        return 42;
    }

    function g(uint256[][1][] calldata a) external returns(uint) {
        a[0];
        return 42;
    }

    function h(uint256[][1][] calldata a) external returns(uint) {
        a[0][0];
        return 42;
    }
}

// ----
// f(uint256[][1][]): 0x20, 0 -> 42
// f(uint256[][1][]):"32, 0" -> "42"
// f(uint256[][1][]): 0x20, 1 -> 
// f(uint256[][1][]):"32, 1" -> ""
// f(uint256[][1][]): 0x20, 1, 0x20 -> 42
// f(uint256[][1][]):"32, 1, 32" -> "42"
// g(uint256[][1][]): 0x20, 1, 0x20 -> 
// g(uint256[][1][]):"32, 1, 32" -> ""
// f(uint256[][1][]): 0x20, 1, 0x20, 0x20 -> 42
// f(uint256[][1][]):"32, 1, 32, 32" -> "42"
// g(uint256[][1][]): 0x20, 1, 0x20, 0x20 -> 42
// g(uint256[][1][]):"32, 1, 32, 32" -> "42"
// h(uint256[][1][]): 0x20, 1, 0x20, 0x20 -> 
// h(uint256[][1][]):"32, 1, 32, 32" -> ""
// f(uint256[][1][]): 0x20, 1, 0x20, 0x20, 1 -> 42
// f(uint256[][1][]):"32, 1, 32, 32, 1" -> "42"
// g(uint256[][1][]): 0x20, 1, 0x20, 0x20, 1 -> 42
// g(uint256[][1][]):"32, 1, 32, 32, 1" -> "42"
// h(uint256[][1][]): 0x20, 1, 0x20, 0x20, 1 -> 
// h(uint256[][1][]):"32, 1, 32, 32, 1" -> ""
