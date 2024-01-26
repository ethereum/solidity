contract C {
    function f() external pure {
        assembly {
            tstore(0, 0)
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// TypeError 8961: (77-89): Function cannot be declared as pure because this expression (potentially) modifies the state.
