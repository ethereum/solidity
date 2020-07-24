contract test {
    enum Choice {A, B, C}

    function answer() public returns (test.Choice _ret) {
        _ret = test.Choice.B;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// answer() -> 1
