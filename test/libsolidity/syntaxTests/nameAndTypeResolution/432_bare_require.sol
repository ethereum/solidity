contract C {
    // This is different because it does have overloads.
    function f() pure public { require; }
}
// ----
// Warning 6133: (101-108): Statement has no effect.
