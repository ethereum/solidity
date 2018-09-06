contract D {}
contract C {
    function f() public view { new D(); }
}
// ----
// TypeError: (58-65): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
