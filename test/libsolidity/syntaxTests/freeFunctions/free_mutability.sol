function f() {
    uint x = 2;
    x;
}
function g(uint[] storage x) pure { x[0] = 1; }
// ----
// Warning 2018: (0-39): Function state mutability can be restricted to pure
// TypeError 8961: (76-80): Function cannot be declared as pure because this expression (potentially) modifies the state.
