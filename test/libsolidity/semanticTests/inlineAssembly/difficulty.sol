contract C {
    function f() public view returns (uint ret) {
        assembly {
            ret := difficulty()
        }
    }
}
// ====
// EVMVersion: <paris
// ----
// f() -> 200000000
