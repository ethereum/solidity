pragma experimental SMTChecker;
contract C {
    function f(address a, function(uint) external g) internal pure {
		address b = g.address;
		assert(a == b);
    }
}
// ----
// Warning 7650: (128-137): Assertion checker does not yet support this expression.
// Warning 4661: (141-155): Assertion violation happens here
