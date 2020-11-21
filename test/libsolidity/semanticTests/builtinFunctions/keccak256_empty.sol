contract C {
    function f() public returns (bytes32) {
        return keccak256("");
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470
