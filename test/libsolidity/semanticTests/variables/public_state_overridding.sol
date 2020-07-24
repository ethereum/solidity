contract A
{
	function test() external virtual returns (uint256)
	{
		return 5;
	}
}
contract X is A
{
	uint256 public override test;

	function set() public { test = 2; }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> 0
// set() ->
// test() -> 2
