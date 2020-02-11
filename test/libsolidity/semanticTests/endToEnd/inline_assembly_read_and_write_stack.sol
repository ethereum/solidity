contract C {
    function f() public returns(uint r) {
        for (uint x = 0; x < 10; ++x)
            assembly {
                r := add(r, x)
            }
    }
}

// ====
// compileViaYul: also
// ----
// f() -> 45
