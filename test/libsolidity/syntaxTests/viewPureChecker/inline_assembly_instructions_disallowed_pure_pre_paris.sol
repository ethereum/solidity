contract C {
    function f() public pure {
        assembly {
            pop(difficulty())
        }
    }
}
// ====
// EVMVersion: <=london
// ----
// TypeError 2527: (79-91): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
