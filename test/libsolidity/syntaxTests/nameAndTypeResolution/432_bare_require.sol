contract C {
    // This is different because it does have overloads.
    function f() pure public { require; }
}
// ----
// TypeError: (101-108): No matching declaration found after variable lookup.
