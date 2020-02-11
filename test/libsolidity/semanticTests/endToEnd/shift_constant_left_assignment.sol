contract C {
    function f() public returns(uint a) {
        a = 0x42;
        a <<= 8;
    }
}

// ----
// f() -> 0x4200
// f():"" -> "16896"
