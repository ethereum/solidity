contract C {
    function f() public view {
        assembly { pop(chainid()) }
    }
}
// ====
// EVMVersion: >=istanbul
// ----
