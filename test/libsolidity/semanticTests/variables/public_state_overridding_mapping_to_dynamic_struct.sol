pragma abicoder               v2;

struct S { uint256 v; string s; }

contract A
{
	function test(uint256 x) external virtual returns (uint256 v, string memory s)
	{
	    v = x;
	    s = "test";
	}
}
contract X is A
{
	mapping(uint256 => S) public override test;

	function set() public { test[42].v = 2; test[42].s = "statevar"; }
}


// ====
// compileViaYul: also
// ----
// test(uint256): 0 -> 0, 64, 0
// test(uint256): 42 -> 0, 64, 0
// set() ->
// test(uint256): 0 -> 0, 64, 0
// test(uint256): 42 -> 2, 0x40, 8, "statevar"
