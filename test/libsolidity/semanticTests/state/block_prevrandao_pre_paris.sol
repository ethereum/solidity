contract C {
    function f() public view returns (uint) {
        return block.prevrandao;
    }
}
// ====
// compileToEwasm: also
// EVMVersion: <paris
// ----
// f() -> 200000000
