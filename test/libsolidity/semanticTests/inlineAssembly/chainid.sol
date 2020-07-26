contract C {
    function f() public returns (uint id) {
        assembly {
            id := chainid()
        }
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// EVMVersion: >=istanbul
// ----
// f() -> 1
