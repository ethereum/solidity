contract test {
    function f(uint k) public returns(uint d) {
        return k;
    }

    function f(uint a, uint b) public returns(uint d) {
        return a + b;
    }

    function g() public returns(uint d) {
        return f(3);
    }
}

// ====
// compileViaYul: also
// ----
// g() -> 3
// g():"" -> "3"
