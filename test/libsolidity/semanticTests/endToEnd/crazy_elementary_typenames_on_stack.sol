contract C {
    function f() public returns(uint r) {
        uint;
        uint;
        uint;
        uint;
        int x = -7;
        return uint(x);
    }
}

// ====
// compileViaYul: also
// ----
// f() -> -7
// f():"" -> "115792089237316195423570985008687907853269984665640564039457584007913129639929"
