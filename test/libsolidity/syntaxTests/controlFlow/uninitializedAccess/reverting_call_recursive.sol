contract C
{
	function iWillRevertLevel2(bool _recurse) pure public
	{
		if (_recurse)
			iWillRevertLevel1();
		else
			revert();
	}

	function iWillRevertLevel1() pure public { iWillRevertLevel2(true); }
	function iWillRevert() pure public { iWillRevertLevel1(); }

	function test(bool _param) pure external returns(uint256)
	{
		if (_param) return 1;

		iWillRevert();
	}
}

// ----
