pragma experimental SMTChecker;

contract C {
	function f(bytes calldata x, uint y) external pure {
		x[8][0];
		x[8][5*y];
	}
}
// ----
// Warning 4984: (118-121): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\ny = 23158417847463239084714197001737581570653996933128112807891516801582625927988\n\n\nTransaction trace:\nconstructor()\nf(x, 23158417847463239084714197001737581570653996933128112807891516801582625927988)
