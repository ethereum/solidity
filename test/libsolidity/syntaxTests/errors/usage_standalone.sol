error E();
function f() pure {
    E();
}
// ----
// TypeError 7757: (35-38): Errors can only be created directly inside require or revert calls.
