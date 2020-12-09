contract C {
    function f() public returns (uint) {
        return tx.gasprice;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 3000000000
// f() -> 3000000000
// f() -> 3000000000
