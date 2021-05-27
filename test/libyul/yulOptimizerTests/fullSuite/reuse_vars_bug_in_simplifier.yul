{
	// This tests that a bug is fixed that was related to just taking
	// values of SSA variables in account and not clearing them when
	// a dependency was reassigned.
	pop(foo())
	function foo() -> x_9
	{
		x_9 := sub(1,sub(x_9,1))
		mstore(sub(1,div(sub(x_9,1),sub(1,sub(x_9,1)))), 1)
	}
}
// ----
// step: fullSuite
//
// { { } }
