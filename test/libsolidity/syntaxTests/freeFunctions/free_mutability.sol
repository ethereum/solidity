function f() {
    uint x = 2;
    x;
}
function g(uint[] storage x) pure { x[0] = 1; }
// ----
// Warning 2018: (0-39): Function state mutability can be restricted to pure
// TypeError 8961: (76-80): Function declared as pure, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
