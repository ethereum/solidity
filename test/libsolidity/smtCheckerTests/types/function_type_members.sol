pragma experimental SMTChecker;
contract C {
    function f(function(uint) external payable g) internal {
		g.selector;
		g.gas(2).value(3)(4);
    }
}
// ----
// Warning: (108-118): Assertion checker does not yet support this expression.
// Warning: (122-130): Assertion checker does not yet implement this type of function call.
// Warning: (122-139): Assertion checker does not yet implement this type of function call.
