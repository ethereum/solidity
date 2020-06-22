pragma experimental SMTChecker;
contract C {
    function f(function(uint) external payable g) internal {
		g.selector;
		g.gas(2).value(3)(4);
		g{gas: 2, value: 3}(4);
    }
}
// ----
// Warning 1621: (122-127): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
// Warning 1621: (122-136): Using ".value(...)" is deprecated. Use "{value: ...}" instead.
// Warning 7650: (108-118): Assertion checker does not yet support this expression.
// Warning 4588: (122-130): Assertion checker does not yet implement this type of function call.
// Warning 4588: (122-139): Assertion checker does not yet implement this type of function call.
