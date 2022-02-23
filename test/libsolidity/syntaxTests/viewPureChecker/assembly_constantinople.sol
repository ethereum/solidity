contract C {
    function f() public view {
        assembly { pop(extcodehash(0)) }
    }
}
// ====
// EVMVersion: >=constantinople
// ----
