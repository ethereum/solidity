contract C {
    function f() public pure {
        assembly {
            // Renamed in paris
            pop(difficulty())
        }
    }
}
// ====
// EVMVersion: =london
// ----
// TypeError 2527: (111-123): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
