contract C {
    function f() public returns (uint) {
        return block.chainid;
    }
}
// ====
// EVMVersion: >=istanbul
// compileViaYul: also
// ----
// f() -> 1
// f() -> 1
// f() -> 1
