contract C {
    function f() public pure {
        assembly {
            pop(blobbasefee())
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// TypeError 2527: (79-92): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
