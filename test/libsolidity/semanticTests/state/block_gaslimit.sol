contract C {
    function f() public returns (uint) {
        return block.gaslimit;
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 20000000
// f() -> 20000000
// f() -> 20000000
