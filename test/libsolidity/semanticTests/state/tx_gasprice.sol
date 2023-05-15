contract C {
    function f() public returns (uint) {
        return tx.gasprice;
    }
}
// ----
// f() -> 3000000000
// f() -> 3000000000
// f() -> 3000000000
