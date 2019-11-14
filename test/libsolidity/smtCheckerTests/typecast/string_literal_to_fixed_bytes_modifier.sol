pragma experimental SMTChecker;
contract B {
    function f() mod2("0123456") pure public { }
    modifier mod2(bytes7 a) {
		assert(a == "0123456");
		assert(a == "1234567");
		_;
	}
}
// ----
// Warning: (152-174): Assertion violation happens here
