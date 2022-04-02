error E();
function f() pure {
    E();
}
// ----
// TypeError 7757: (35-38='E()'): Errors can only be used with revert statements: "revert MyError();".
