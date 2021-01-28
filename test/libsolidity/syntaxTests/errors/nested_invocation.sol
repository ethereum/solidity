error E1();
error E2();
function f() pure {
    revert(E1(E2));
}
// ----
// TypeError 6160: (55-61): Wrong argument count for function call: 1 arguments given but expected 0.
