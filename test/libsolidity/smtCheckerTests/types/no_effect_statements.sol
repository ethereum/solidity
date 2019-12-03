pragma experimental SMTChecker;
contract test {
 struct s { uint a; uint b;}
    function f() pure public returns (byte) {
		s;
		s(1,2);
		s[7];
		uint;
		uint[7];
    }
}
// ----
// Warning: (125-126): Statement has no effect.
// Warning: (130-136): Statement has no effect.
// Warning: (140-144): Statement has no effect.
// Warning: (148-152): Statement has no effect.
// Warning: (156-163): Statement has no effect.
// Warning: (125-126): Assertion checker does not yet implement type type(struct test.s storage pointer)
// Warning: (130-131): Assertion checker does not yet implement type type(struct test.s storage pointer)
// Warning: (130-136): Assertion checker does not yet implement type struct test.s memory
// Warning: (130-136): Assertion checker does not yet implement this expression.
// Warning: (140-141): Assertion checker does not yet implement type type(struct test.s storage pointer)
// Warning: (140-144): Assertion checker does not yet implement type type(struct test.s memory[7] memory)
// Warning: (156-163): Assertion checker does not yet implement type type(uint256[7] memory)
