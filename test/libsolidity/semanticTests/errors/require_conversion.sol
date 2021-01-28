error E(string a, uint[] b);
contract C {
    uint[] x;
    function f(bool c) public {
        x.push(7);
        require(c, E("abc", x));
    }
}
// ====
// compileViaYul: also
// ----
// f(bool): false -> FAILURE, hex"59e4d4df", 0x40, 0x80, 3, "abc", 1, 7
// f(bool): true ->
