contract A {
	uint[] a;
	function f() public {
		A.a.push(2);
		assert(A.a[A.a.length - 1] == 2);
		A.a.pop();
		// Fails
		assert(A.a.length > 0);
		assert(A.a.length == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (124-146): CHC: Assertion violation happens here.
// Warning 6328: (150-173): CHC: Assertion violation might happen here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// Warning 4661: (150-173): BMC: Assertion violation happens here.
