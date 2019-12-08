pragma experimental SMTChecker;
contract C {
    function f(address a, function(uint) external g) internal pure {
		address b = address(g);
		assert(a == b);
    }
}
// ----
// Warning: (128-138): Type conversion is not yet fully supported and might yield false positives.
// Warning: (142-156): Assertion violation happens here
