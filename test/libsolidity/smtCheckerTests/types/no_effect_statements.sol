contract test {
    struct s { uint a; uint b;}
    function f() pure public returns (bytes1) {
		s;
		s(1,2);
		s[7];
		uint;
		uint[7];
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6321: (86-92): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6133: (98-99): Statement has no effect.
// Warning 6133: (103-109): Statement has no effect.
// Warning 6133: (113-117): Statement has no effect.
// Warning 6133: (121-125): Statement has no effect.
// Warning 6133: (129-136): Statement has no effect.
// Warning 8364: (113-117): Assertion checker does not yet implement type type(struct test.s memory[7] memory)
// Warning 8364: (129-136): Assertion checker does not yet implement type type(uint256[7] memory)
