contract C {
    function f() public pure {
        assembly {
            pop(basefee())
        }
    }
}
// ====
// EVMVersion: >=london
// ----
// TypeError 2527: (79-88): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
