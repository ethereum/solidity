contract C {
    modifier repeat(uint count) {
        uint i;
        for (i = 0; i < count; ++i) _;
    }

    function f() repeat(10) public returns(uint r) {
        r += 1;
    }
}

// ----
// f() -> 10
// f():"" -> "10"
