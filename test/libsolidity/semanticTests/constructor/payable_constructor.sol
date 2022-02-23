contract C {
    constructor() payable {}
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// constructor(), 27 wei ->
