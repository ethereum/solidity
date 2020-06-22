pragma experimental SMTChecker;
contract B {
    function f() pure public {
		g("0123456");
	}
    function g(bytes7 a) pure public {
		assert(a == "0123456");
		assert(a == "1234567");
	}
}
// ----
// Warning 4661: (162-184): Assertion violation happens here
// Warning 4661: (162-184): Assertion violation happens here
