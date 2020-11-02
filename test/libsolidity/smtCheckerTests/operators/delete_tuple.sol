pragma experimental SMTChecker;

contract A{
	function f() public pure {
		delete ([""][0]);
	}
}
