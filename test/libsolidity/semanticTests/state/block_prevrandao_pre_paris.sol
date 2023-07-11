contract C {
    function f() public view returns (uint) {
        return block.prevrandao;
    }
}
// ====
// EVMVersion: <paris
// ----
// f() -> 200000000
