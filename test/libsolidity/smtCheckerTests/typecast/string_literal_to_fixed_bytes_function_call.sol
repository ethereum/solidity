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
// Warning 6328: (162-184): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nB.constructor()\nB.f()
// Warning 6328: (136-158): CHC: Assertion violation happens here.\nCounterexample:\n\na = 13564890559296823\n\n\nTransaction trace:\nB.constructor()\nB.g(13564890559296823)
