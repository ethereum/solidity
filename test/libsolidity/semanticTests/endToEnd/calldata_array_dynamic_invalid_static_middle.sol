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
// f(uint256[][1][]): 0x20, 1 -> FAILURE
// f(uint256[][1][]): 0x20, 1, 0x20 -> 42
// g(uint256[][1][]): 0x20, 1, 0x20 -> FAILURE
// f(uint256[][1][]): 0x20, 1, 0x20, 0x20 -> 42
// g(uint256[][1][]): 0x20, 1, 0x20, 0x20 -> 42
// h(uint256[][1][]): 0x20, 1, 0x20, 0x20 -> FAILURE
// f(uint256[][1][]): 0x20, 1, 0x20, 0x20, 1 -> 42
// g(uint256[][1][]): 0x20, 1, 0x20, 0x20, 1 -> 42
// h(uint256[][1][]): 0x20, 1, 0x20, 0x20, 1 -> FAILURE
