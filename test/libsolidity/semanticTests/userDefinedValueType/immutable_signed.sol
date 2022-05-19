type MyInt is int16;
type MyBytes is bytes2;
contract C {
    MyInt immutable a = MyInt.wrap(-2);
    MyBytes immutable b = MyBytes.wrap("ab");
    function() internal returns (uint) immutable f = g;
    function direct() view external returns (MyInt, MyBytes) {
        return (a, b);
    }
    function viaasm() view external returns (bytes32 x, bytes32 y) {
        MyInt _a = a;
        MyBytes _b = b;
        assembly { x := _a y := _b }
    }
    function g() internal pure returns (uint) { return 2; }
}
// ----
// direct() -> -2, 0x6162000000000000000000000000000000000000000000000000000000000000
// viaasm() -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x6162000000000000000000000000000000000000000000000000000000000000
