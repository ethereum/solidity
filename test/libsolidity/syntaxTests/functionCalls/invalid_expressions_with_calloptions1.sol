contract D {}
contract C {
    function foo(int a) pure internal {
		  a{val:5};
    }
}
// ----
// TypeError: (71-79): Expected callable expression before call options.
