pragma experimental SMTChecker;
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
// ----
// Warning 6321: (118-124): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6133: (130-131): Statement has no effect.
// Warning 6133: (135-141): Statement has no effect.
// Warning 6133: (145-149): Statement has no effect.
// Warning 6133: (153-157): Statement has no effect.
// Warning 6133: (161-168): Statement has no effect.
// Warning 8364: (145-149): Assertion checker does not yet implement type type(struct test.s memory[7] memory)
// Warning 8364: (161-168): Assertion checker does not yet implement type type(uint256[7] memory)
// Warning 8364: (145-149): Assertion checker does not yet implement type type(struct test.s memory[7] memory)
// Warning 8364: (161-168): Assertion checker does not yet implement type type(uint256[7] memory)
