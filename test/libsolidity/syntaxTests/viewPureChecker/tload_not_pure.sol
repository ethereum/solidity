contract C {
    function f() external pure {
        assembly {
            pop(tload(0))
        }
    }
}
// ----
// TypeError 2527: (81-89): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
