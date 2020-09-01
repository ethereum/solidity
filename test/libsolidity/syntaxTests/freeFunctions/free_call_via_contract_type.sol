contract C {
    function f() public pure {}
}
function fun() {
    C.f();
}
// ----
// TypeError 3419: (68-73): Cannot call function via contract type name.
