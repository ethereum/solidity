contract Test {
	struct RecursiveStruct {
		uint x;
		RecursiveStruct[] vals;
	}
	function func() public pure {
		RecursiveStruct memory val = RecursiveStruct(1, new RecursiveStruct[](42));
		assert(val.x == 1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8115: (114-140): Assertion checker does not yet support the type of this variable.
// Warning 8364: (143-188): Assertion checker does not yet implement type struct Test.RecursiveStruct memory
// Warning 7650: (199-204): Assertion checker does not yet support this expression.
// Warning 8364: (199-202): Assertion checker does not yet implement type struct Test.RecursiveStruct memory
// Warning 6328: (192-210): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nTest.constructor()\nTest.func()
// Warning 8115: (114-140): Assertion checker does not yet support the type of this variable.
// Warning 8364: (143-188): Assertion checker does not yet implement type struct Test.RecursiveStruct memory
// Warning 7650: (199-204): Assertion checker does not yet support this expression.
// Warning 8364: (199-202): Assertion checker does not yet implement type struct Test.RecursiveStruct memory
