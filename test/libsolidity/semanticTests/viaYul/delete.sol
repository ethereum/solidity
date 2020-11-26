contract C {
	function internal_func() internal pure returns (int8)
	{
		return 1;
	}
	function call_internal_func() public pure returns (bool ret)
	{
		function() internal pure returns(int8) func = internal_func;

		return func() == internal_func();
	}
	function call_deleted_internal_func() public pure returns (bool ret)
	{
		function() internal pure returns(int8) func = internal_func;

		delete func;

		return func() == internal_func();
	}
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// call_deleted_internal_func() -> FAILURE, hex"4e487b71", 0x51
// call_internal_func() -> true
