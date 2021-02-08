error E();
function f() pure {
    revert((E)());
}
// ----
// TypeError 4423: (42-47): Expected error instance or string.
