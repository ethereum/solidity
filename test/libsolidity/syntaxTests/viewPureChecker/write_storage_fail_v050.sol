pragma experimental "v0.5.0";
contract C {
    uint x;
    function f() view public { x = 2; }
}
// ----
// TypeError: (86-87): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
