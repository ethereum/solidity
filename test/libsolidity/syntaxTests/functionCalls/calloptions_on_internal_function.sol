contract D {}
contract C {
    function foo(int a) pure internal {
        foo{gas: 5};
    }
}
// ----
// TypeError 2193: (75-86='foo{gas: 5}'): Function call options can only be set on external function calls or contract creations.
