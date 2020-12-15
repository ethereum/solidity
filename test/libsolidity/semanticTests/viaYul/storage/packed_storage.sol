contract C {
    uint16 x;
    bytes1 y;
    uint16 z;
    function f(uint8 a) public returns (uint _x) {
        x = a;
        y = bytes1(uint8(x) + 1);
        z = uint8(y) + 1;
        x = z + 1;
        _x = x;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(uint8): 6 -> 9
