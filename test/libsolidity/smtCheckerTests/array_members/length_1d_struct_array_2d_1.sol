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
// Warning: (78-82): Assertion checker does not yet support the type of this variable.
// Warning: (85-89): Assertion checker does not yet support the type of this variable.
// Warning: (128-134): Assertion checker does not yet support this expression.
// Warning: (128-130): Assertion checker does not yet implement type struct C.S storage ref
// Warning: (128-137): Assertion checker does not yet implement this expression.
// Warning: (148-154): Assertion checker does not yet support this expression.
// Warning: (148-150): Assertion checker does not yet implement type struct C.S storage ref
// Warning: (148-157): Assertion checker does not yet implement this expression.
// Warning: (121-165): Assertion violation happens here
