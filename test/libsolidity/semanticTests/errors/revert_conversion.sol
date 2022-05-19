error E(string a, uint[] b);
contract C {
    uint[] x;
    function f() public {
        x.push(7);
        revert E("abc", x);
    }
}
// ----
// f() -> FAILURE, hex"59e4d4df", 0x40, 0x80, 3, "abc", 1, 7
