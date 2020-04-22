contract C {
    constructor() public payable {}
}
contract D is C {}
// ====
// compileViaYul: also
// ----
// constructor(), 27 wei ->