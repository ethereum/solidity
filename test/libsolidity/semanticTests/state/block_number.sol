contract C {
    constructor() {}
    function f() public returns (uint) {
        return block.number;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// constructor()
// f() -> 2
// f() -> 3
