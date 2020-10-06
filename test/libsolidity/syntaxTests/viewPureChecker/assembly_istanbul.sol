contract C {
    function f() public pure {
        assembly { pop(chainid()) }
    }
}
// ====
// EVMVersion: >=istanbul
// ----
