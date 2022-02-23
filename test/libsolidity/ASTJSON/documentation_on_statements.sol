contract C {
    // this is not exported
    uint a;
    function f() public pure returns (uint x) {
        // test2
        for (uint i = 0; i < 20; i++) {
            // not exported either
            x *= 2;
        }
        // nor is this because they are all
        // not using the triple-slash
        return x;
    }
}

// ----
