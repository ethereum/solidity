contract test {
    function f() public returns (bool) {
        int256 x = -2**255;
        assert(-x == x);
        return true;
    }
}

// ----
// f() -> true
