pragma abicoder               v2;

struct S { uint256 v; string s; }

contract A
{
	function test() external virtual returns (uint256 v, string memory s)
	{
	    v = 42;
	    s = "test";
	}
}
contract X is A
{
	S public override test;

	function set() public { test.v = 2; test.s = "statevar"; }
}
// ----
// test() -> 0, 64, 0
// set() ->
// test() -> 2, 0x40, 8, "statevar"
