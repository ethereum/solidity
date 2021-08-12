contract D { constructor() payable {} }
contract C {
	uint public x;
	constructor() payable {}
	function f(uint amount) public returns (D) {
		x++;
		return (new D){value: amount}();
	}
	function stack(uint depth) public payable returns (address) {
		if (depth > 0)
			return this.stack(depth - 1);
		else
			return address(f(0));
	}
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// ----
// constructor(), 20 wei
// gas irOptimized: 220113
// gas legacy: 288299
// gas legacyOptimized: 174543
// f(uint256): 20 -> 1370859564726510389319704988634906228201275401179
// x() -> 1
// f(uint256): 20 -> FAILURE
// x() -> 1
// stack(uint256): 1023 -> FAILURE
// gas irOptimized: 345821
// gas legacy: 535367
// gas legacyOptimized: 349920
// x() -> 1
// stack(uint256): 10 -> 693016686122178122849713379390321835634789309880
// x() -> 2
