pragma abicoder v2;

type MyUInt8 is uint8;
type MyInt8 is int8;
type MyUInt16 is uint16;

contract C {
    function f(uint a) external returns(MyUInt8) {
        return MyUInt8.wrap(uint8(a));
    }
    function g(uint a) external returns(MyInt8) {
        return MyInt8.wrap(int8(int(a)));
    }
    function h(MyUInt8 a) external returns (MyInt8) {
        return MyInt8.wrap(int8(MyUInt8.unwrap(a)));
    }
    function i(MyUInt8 a) external returns(MyUInt16) {
        return MyUInt16.wrap(MyUInt8.unwrap(a));
    }
    function j(MyUInt8 a) external returns (uint) {
        return MyUInt8.unwrap(a);
    }
    function k(MyUInt8 a) external returns (MyUInt16) {
        return MyUInt16.wrap(MyUInt8.unwrap(a));
    }
    function m(MyUInt16 a) external returns (MyUInt8) {
        return MyUInt8.wrap(uint8(MyUInt16.unwrap(a)));
    }
}

// ----
// f(uint256): 1 -> 1
// f(uint256): 2 -> 2
// f(uint256): 257 -> 1
// g(uint256): 1 -> 1
// g(uint256): 2 -> 2
// g(uint256): 255 -> -1
// g(uint256): 257 -> 1
// h(uint8): 1 -> 1
// h(uint8): 2 -> 2
// h(uint8): 255 -> -1
// h(uint8): 257 -> FAILURE
// i(uint8): 250 -> 250
// j(uint8): 1 -> 1
// j(uint8): 2 -> 2
// j(uint8): 255 -> 0xff
// j(uint8): 257 -> FAILURE
// k(uint8): 1 -> 1
// k(uint8): 2 -> 2
// k(uint8): 255 -> 0xff
// k(uint8): 257 -> FAILURE
// m(uint16): 1 -> 1
// m(uint16): 2 -> 2
// m(uint16): 255 -> 0xff
// m(uint16): 257 -> 1
