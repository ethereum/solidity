contract C {
    uint x;
    function f() view public { x = 2; }
}
// ----
// Warning: (56-57): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
