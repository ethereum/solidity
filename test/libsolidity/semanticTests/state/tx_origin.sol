contract C {
    function f() public returns (address payable) {
        return tx.origin;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x9292929292929292929292929292929292929292
// f() -> 0x9292929292929292929292929292929292929292
// f() -> 0x9292929292929292929292929292929292929292
