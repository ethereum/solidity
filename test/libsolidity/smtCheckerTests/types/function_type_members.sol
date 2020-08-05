pragma experimental SMTChecker;
contract C {
    function f(function(uint) external payable g) internal {
		g.selector;
		g{gas: 2, value: 3}(4);
    }
}
// ----
// Warning 7650: (108-118): Assertion checker does not yet support this expression.
