contract C {
    function f() public returns (uint) {
        return block.difficulty;
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 200000000
// f() -> 200000000
// f() -> 200000000
