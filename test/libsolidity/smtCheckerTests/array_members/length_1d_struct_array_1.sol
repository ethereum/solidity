pragma experimental SMTChecker;

contract C {
	struct S {
		uint[] arr;
	}
	S s1;
	S s2;
	function f() public view {
		assert(s1.arr.length == s2.arr.length);
	}
}
// ----
// Warning 6328: (119-157): Assertion violation happens here
// Warning 8115: (76-80): Assertion checker does not yet support the type of this variable.
// Warning 8115: (83-87): Assertion checker does not yet support the type of this variable.
// Warning 7650: (126-132): Assertion checker does not yet support this expression.
// Warning 8364: (126-128): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (143-149): Assertion checker does not yet support this expression.
// Warning 8364: (143-145): Assertion checker does not yet implement type struct C.S storage ref
