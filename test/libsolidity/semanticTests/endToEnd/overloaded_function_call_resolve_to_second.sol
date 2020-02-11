contract test {
    function f(uint a, uint b) public returns(uint d) {
        return a + b;
    }

    function f(uint k) public returns(uint d) {
        return k;
    }

    function g() public returns(uint d) {
        return f(3, 7);
    }
}

// ====
// compileViaYul: also
// ----
// g() -> 10
