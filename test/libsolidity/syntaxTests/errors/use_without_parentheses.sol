error E1();
error E2();
function f() pure {
    revert(E1);
}
// ----
// TypeError 4423: (55-57): Expected error or string.
