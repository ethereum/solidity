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
// ----
// constructor(): 3 ->
// gas irOptimized: 125477
// gas legacy: 209361
// gas legacyOptimized: 139324
// f() -> 84, 23
// m(uint256): 3 -> 7
