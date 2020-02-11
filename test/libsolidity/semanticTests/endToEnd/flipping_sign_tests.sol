contract test {
    function f() public returns(bool) {
        int x = -2 ** 255;
        assert(-x == x);
        return true;
    }
}

// ----
// f() -> true
// f():"" -> "1"
