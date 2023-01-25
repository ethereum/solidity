contract C {
    function f() public view returns (uint ret) {
        assembly {
            ret := difficulty()
        }
    }
}
// ====
// compileToEwasm: also
// EVMVersion: <paris
// ----
// f() -> 200000000
