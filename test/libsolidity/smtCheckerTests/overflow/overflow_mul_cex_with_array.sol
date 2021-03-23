pragma experimental SMTChecker;

contract C {
	function f(bytes calldata x, uint y) external pure {
		require(x.length > 10);
		x[8][0];
		x[8][5*y];
	}
}
// ----
// Warning 4984: (144-147): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\n\nx = [38, 38, 38, 38, 38, 38, 9, 38, 38, 38, 13]\ny = 23158417847463239084714197001737581570653996933128112807891516801582625927988\n\nTransaction trace:\nC.constructor()\nC.f([38, 38, 38, 38, 38, 38, 9, 38, 38, 38, 13], 23158417847463239084714197001737581570653996933128112807891516801582625927988)
// Warning 6368: (139-148): CHC: Out of bounds access happens here.\nCounterexample:\n\nx = [38, 38, 38, 38, 38, 38, 38, 9, 38, 38, 38]\ny = 1\n\nTransaction trace:\nC.constructor()\nC.f([38, 38, 38, 38, 38, 38, 38, 9, 38, 38, 38], 1)
