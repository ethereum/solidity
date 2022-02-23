contract Test {
	struct RecursiveStruct {
		RecursiveStruct[] vals;
	}
	function func() public pure {
		RecursiveStruct[1] memory val = [ RecursiveStruct(new RecursiveStruct[](42)) ];
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (104-133): Unused local variable.
// Warning 8364: (138-180): Assertion checker does not yet implement type struct Test.RecursiveStruct memory
