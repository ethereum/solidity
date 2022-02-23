library L
{
	function iWillRevert() public pure { revert(); }
}

contract C
{
	function test(bool _param) pure external returns(uint256)
	{
		if (_param) return 1;

		L.iWillRevert();
	}
}

// ----
// Warning 6321: (128-135): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
