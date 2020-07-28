pragma experimental SMTChecker;
contract c {
	bytes10[6] data2;
	function test() public view returns (bytes10 r2) {
		r2 = data2[4][5];
	}
}
// ----
// Warning 7989: (123-134): Assertion checker does not yet support index accessing fixed bytes.
