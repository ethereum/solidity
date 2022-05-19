contract test {
    function f() public returns (bool) {
        int256 x = -2**255;
        unchecked { assert(-x == x); }
        return true;
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> true
