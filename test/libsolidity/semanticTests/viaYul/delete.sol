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
	function external_func() external pure returns (int8)
	{
		return 1;
	}
}
// ====
// compileViaYul: true
// ----
// call_deleted_internal_func() -> FAILURE
// call_internal_func() -> true
