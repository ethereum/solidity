contract C {
    constructor() payable {}
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// constructor(), 27 wei ->
