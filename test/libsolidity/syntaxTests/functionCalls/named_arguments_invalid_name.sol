contract test {
    function f(uint a, bool b, bytes memory c, uint d, bool e) public returns (uint r) {
        if (b && !e)
            r = a + d;
        else
            r = c.length;
    }
    function g() public returns (uint r) {
        r = f({c: "abc", x: 1, e: 2, a: 11, b: 12});
    }
}
// ----
// TypeError 4974: (249-288): Named argument "x" does not match function declaration.
