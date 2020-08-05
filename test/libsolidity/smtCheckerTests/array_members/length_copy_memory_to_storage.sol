pragma experimental SMTChecker;

contract C {
	uint[] arr;
	function f(uint[] memory marr) public {
		arr = marr;
		assert(marr.length == arr.length);
	}
}
