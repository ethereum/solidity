pragma abicoder v2;
type MyUInt8 is uint8;

// Note that this wraps from a uint256
function wrap(uint x) pure returns (MyUInt8 y) { assembly { y := x } }
function unwrap(MyUInt8 x) pure returns (uint8 y) { assembly { y := x } }

contract C {
    uint8 a;
    MyUInt8 b;
    uint8 c;
    function ret() external returns(MyUInt8) {
        return wrap(0x1ff);
    }
    function f(MyUInt8 x) external returns(MyUInt8) {
        return x;
    }
    function mem() external returns (MyUInt8[] memory) {
        MyUInt8[] memory x = new MyUInt8[](2);
        x[0] = wrap(0x1ff);
        x[1] = wrap(0xff);
        require(unwrap(x[0]) == unwrap(x[1]));
        assembly {
            mstore(add(x, 0x20), 0x1ff)
        }
        require(unwrap(x[0]) == unwrap(x[1]));
        return x;
    }
    function stor() external returns (uint8, MyUInt8, uint8) {
        a = 1;
        c = 2;
        b = wrap(0x1ff);
        return (a, b, c);
    }
}
// ----
// ret() -> 0xff
// f(uint8): 0x1ff -> FAILURE
// f(uint8): 0xff -> 0xff
// mem() -> 0x20, 2, 0xff, 0xff
// stor() -> 1, 0xff, 2
