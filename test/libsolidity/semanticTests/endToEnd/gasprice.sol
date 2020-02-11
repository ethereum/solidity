contract C {
    function f() public returns(uint) {
        return tx.gasprice;
    }
}

// ====
// compileViaYul: also
// ----
// f() -> gasPrice(
// f():"" -> "0"
