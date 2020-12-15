pragma experimental SMTChecker;
contract Test {
	struct RecursiveStruct {
		RecursiveStruct[] vals;
	}
	function func() public pure {
		RecursiveStruct[1] memory val = [ RecursiveStruct(new RecursiveStruct[](42)) ];
	}
}
// ----
// Warning 2072: (136-165): Unused local variable.
// Warning 8364: (170-212): Assertion checker does not yet implement type struct Test.RecursiveStruct memory
// Warning 8364: (170-212): Assertion checker does not yet implement type struct Test.RecursiveStruct memory
