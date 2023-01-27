contract C {
    function f() public pure {
        assembly {
            pop(extcodehash(0))
            pop(create2(0, 1, 2, 3))
        }
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// TypeError 2527: (79-93): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (111-130): Function cannot be declared as pure because this expression (potentially) modifies the state.
