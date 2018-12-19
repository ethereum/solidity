pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f() public {
		map[1] = 111;
		uint x = map[2];
		map[1] = 112;
		assert(map[2] == x);
	}
}
