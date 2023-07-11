contract C {
    function f() public returns (uint) {
        return block.difficulty;
    }
}
// ====
// EVMVersion: <paris
// ----
// f() -> 200000000
// f() -> 200000000
// f() -> 200000000
