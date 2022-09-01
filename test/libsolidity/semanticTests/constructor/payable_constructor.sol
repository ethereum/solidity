contract C {
    constructor() payable {}
}

// ====
// compileToEwasm: also
// ----
// constructor(), 27 wei ->
