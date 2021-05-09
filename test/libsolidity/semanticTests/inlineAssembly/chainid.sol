contract C {
    function f() public returns (uint id) {
        assembly {
            id := chainid()
        }
    }
}
// ====
// EVMVersion: >=istanbul
// compileViaYul: also
// ----
// f() -> 1
