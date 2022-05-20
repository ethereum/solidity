contract C {
    function f() public returns (uint id) {
        assembly {
            id := chainid()
        }
    }
}
// ====
// EVMVersion: >=istanbul
// ----
// f() -> 1
