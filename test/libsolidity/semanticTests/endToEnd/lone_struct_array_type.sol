contract C {
    struct s {
        uint a;
        uint b;
    }

    function f() public returns(uint) {
        s[7][]; // This is only the type, should not have any effect
        return 3;
    }
}

// ====
// compileViaYul: also
// ----
// f() -> 3
// f():"" -> "3"
