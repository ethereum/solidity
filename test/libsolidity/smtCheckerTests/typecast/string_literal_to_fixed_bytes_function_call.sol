contract B {
    function f() pure public {
		g("0123456");
	}
    function g(bytes7 a) pure public {
		assert(a == "0123456");
		assert(a == "1234567");
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (104-126): CHC: Assertion violation happens here.
// Warning 6328: (130-152): CHC: Assertion violation happens here.
