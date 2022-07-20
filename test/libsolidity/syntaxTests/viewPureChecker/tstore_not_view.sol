contract C {
    function f() external view {
        assembly {
            tstore(0, 0)
        }
    }
}
// ----
// TypeError 8961: (77-89): Function cannot be declared as view because this expression (potentially) modifies the state.
