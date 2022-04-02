contract D {}
contract C {
    function foo(int a) pure internal {
		  a{val:5};
    }
}
// ----
// TypeError 2622: (71-79='a{val:5}'): Expected callable expression before call options.
