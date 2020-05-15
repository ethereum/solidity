pragma experimental SMTChecker;

contract C {
	mapping (uint => uint[]) map;
	function f() public view {
		assert(map[0].length == map[1].length);
	}
}
