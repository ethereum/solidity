contract C {
    function f() public returns (address) {
        return tx.origin;
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 0x9292929292929292929292929292929292929292
// f() -> 0x9292929292929292929292929292929292929292
// f() -> 0x9292929292929292929292929292929292929292
