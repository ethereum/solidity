contract C {
    constructor() {}
    function f() public returns (uint) {
        return block.number;
    }
}
// ====
// compileViaYul: also
// ----
// constructor()
// gas ir: 130626
// gas legacy: 100281
// f() -> 2
// f() -> 3
