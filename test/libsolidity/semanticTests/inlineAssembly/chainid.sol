contract C {
    function f() public returns (uint id) {
        assembly {
            id := chainid()
        }
    }
}
// ====
// compileViaYul: also
// EVMVersion: >=istanbul
// ----
// f() -> 1
