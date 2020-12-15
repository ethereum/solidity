pragma experimental SMTChecker;

contract C {
	struct S {
		int[][] d;
	}
	S[] data;
	function f() public {
		++data[1].d[3].push();
	}
}
