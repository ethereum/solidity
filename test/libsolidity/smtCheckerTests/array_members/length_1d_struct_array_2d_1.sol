pragma experimental SMTChecker;

contract C {
	struct S {
		uint[][] arr;
	}
	S s1;
	S s2;
	function f() public view {
		assert(s1.arr[0].length == s2.arr[0].length);
	}
}
// ----
// Warning 6328: (121-165): Assertion violation happens here
// Warning 8115: (78-82): Assertion checker does not yet support the type of this variable.
// Warning 8115: (85-89): Assertion checker does not yet support the type of this variable.
// Warning 7650: (128-134): Assertion checker does not yet support this expression.
// Warning 8364: (128-130): Assertion checker does not yet implement type struct C.S storage ref
// Warning 9118: (128-137): Assertion checker does not yet implement this expression.
// Warning 7650: (148-154): Assertion checker does not yet support this expression.
// Warning 8364: (148-150): Assertion checker does not yet implement type struct C.S storage ref
// Warning 9118: (148-157): Assertion checker does not yet implement this expression.
