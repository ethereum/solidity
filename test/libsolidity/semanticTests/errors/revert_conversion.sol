error E(string a, uint[] b);
contract C {
    uint[] x;
    function f() public {
        x.push(7);
        revert E("abc", x);
    }
    function encode_f() public returns (bytes memory) {
        x.push(7);
        return abi.encodeError(E, ("abc", x));
    }
}
// ----
// f() -> FAILURE, hex"59e4d4df", 0x40, 0x80, 3, "abc", 1, 7
// encode_f() -> 0x20, 0xc4, hex"59e4d4df", 0x40, 0x80, 3, "abc", 1, 7, hex"00000000000000000000000000000000000000000000000000000000"
