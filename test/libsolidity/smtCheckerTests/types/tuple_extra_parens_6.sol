pragma experimental SMTChecker;
contract C {
	function f() public pure {
		(((,))) = ((2),3);
	}
}
