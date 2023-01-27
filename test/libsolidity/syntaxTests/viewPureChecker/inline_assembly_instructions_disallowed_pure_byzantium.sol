contract C {
    function f() public pure {
        assembly {
            pop(staticcall(0, 1, 2, 3, 4, 5))
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError 2527: (79-107): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
