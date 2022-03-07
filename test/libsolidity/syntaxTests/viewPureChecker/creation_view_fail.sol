contract D {}
contract C {
    function f() public view { new D(); }
}
// ----
// TypeError 8961: (58-65): Function cannot be declared as view because this expression (potentially) modifies the state.
