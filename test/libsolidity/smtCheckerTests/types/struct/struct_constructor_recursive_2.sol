pragma experimental SMTChecker;
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
// ----
// Warning 8115: (146-172): Assertion checker does not yet support the type of this variable.
// Warning 8364: (175-220): Assertion checker does not yet implement type struct Test.RecursiveStruct memory
// Warning 7650: (231-236): Assertion checker does not yet support this expression.
// Warning 8364: (231-234): Assertion checker does not yet implement type struct Test.RecursiveStruct memory
// Warning 6328: (224-242): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nfunc()
// Warning 8115: (146-172): Assertion checker does not yet support the type of this variable.
// Warning 8364: (175-220): Assertion checker does not yet implement type struct Test.RecursiveStruct memory
// Warning 7650: (231-236): Assertion checker does not yet support this expression.
// Warning 8364: (231-234): Assertion checker does not yet implement type struct Test.RecursiveStruct memory
