contract C {
    function f() public returns (uint256 r) {
        for (uint256 x = 0; x < 10; ++x)
            assembly {
                r := add(r, x)
            }
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> 45
