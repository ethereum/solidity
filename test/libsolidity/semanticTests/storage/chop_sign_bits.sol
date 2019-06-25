contract Test {
    int16[] public x = [-1, -2];
    int16[2] public y = [-5, -6];
    int16 z;
    function f() public returns (int16[] memory) {
        int8[] memory t = new int8[](2);
        t[0] = -3;
        t[1] = -4;
        x = t;
        return x;
    }
    function g() public returns (int16[2] memory) {
        int8[2] memory t = [-3, -4];
        y = t;
        return y;
    }
    function h(int8 t) public returns (int16) {
        z = t;
        return z;
    }
}
// ----
// x(uint256): 0 -> -1
// x(uint256): 1 -> -2
// y(uint256): 0 -> -5
// y(uint256): 1 -> -6
// f() -> 0x20, 2, -3, -4
// g() -> -3, -4
// h(int8): -10 -> -10
