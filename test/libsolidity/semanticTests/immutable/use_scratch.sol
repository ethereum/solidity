contract C {
	uint256 immutable x;
	uint256 immutable y;
	mapping(uint => uint) public m;
	constructor(uint _a) {
		x = 42;
		y = 23;
		m[_a] = 7;
		new uint[](4);

	}
	function f() public view returns (uint256, uint256) {
		return (x+x,y);
	}
}
// ====
// compileViaYul: also
// ----
// constructor(): 3 ->
// gas ir: 287339
// gas irOptimized: 170188
// gas legacy: 224559
// gas legacyOptimized: 161211
// f() -> 84, 23
// m(uint256): 3 -> 7
