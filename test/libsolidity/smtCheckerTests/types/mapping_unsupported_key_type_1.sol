pragma experimental SMTChecker;

contract C
{
	mapping (string => uint) map;
	function f(string memory s, uint x) public {
		map[s] = x;
		assert(x == map[s]);
	}
}
// ----
