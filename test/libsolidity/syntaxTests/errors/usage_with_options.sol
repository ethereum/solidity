error E1();
function f() pure {
    revert(E1{gas: 10}());
}
// ----
// TypeError 2193: (43-54): Function call options can only be set on external function calls or contract creations.
