contract C {
    function f() public pure {
        assembly { pop(blobhash(0)) }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// TypeError 2527: (67-78): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
