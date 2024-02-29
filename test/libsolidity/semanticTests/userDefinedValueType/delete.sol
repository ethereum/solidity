type MyUInt256 is uint256;
type MyBytes32 is bytes32;
type MyAddress is address;
type MyBool is bool;

contract C {
    MyUInt256 a = MyUInt256.wrap(255);

    function f() external returns(MyUInt256) {
        delete a;
        return a;
    }

    function g(MyUInt256 b) external returns(MyUInt256) {
        delete b;
        return b;
    }

    function h(MyAddress b) external returns(MyAddress) {
        delete b;
        return b;
    }

    function i(MyBytes32 b) external returns(MyBytes32) {
        delete b;
        return b;
    }

    function j(MyBool b) external returns(MyBool) {
        delete b;
        return b;
    }
}
// ----
// f() -> 0
// g(uint256): 255 -> 0
// g(uint256): 0 -> 0
// h(address): 0xffffffffffffffffffffffffffffffffffffffff -> 0x0000000000000000000000000000000000000000
// h(address): 0x0000000000000000000000000000000000000000 -> 0x0000000000000000000000000000000000000000
// i(bytes32): 0xffffffffffffffffffffffffffffffffffffffff -> 0x0000000000000000000000000000000000000000
// i(bytes32): 0x0000000000000000000000000000000000000000 -> 0x0000000000000000000000000000000000000000
// j(bool): true -> false
// j(bool): false -> false