contract C {
    constructor() public payable {}
}

// ====
// compileViaYul: also
// ----
// constructor(), 27 wei ->
