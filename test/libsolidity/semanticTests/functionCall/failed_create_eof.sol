contract D { constructor() payable {} }
contract C {
	uint public x;
	constructor() payable {}
	function f(uint amount) public returns (D) {
		x++;
		return (new D){value: amount, salt: bytes32(x)}();
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
// compileToEOF: true
// EVMVersion: >=prague
// ----
// constructor(), 20 wei
// gas irOptimized: 61548
// gas irOptimized code: 104600
// gas legacy: 70147
// gas legacy code: 215400
// gas legacyOptimized: 61715
// gas legacyOptimized code: 106800
// f(uint256): 20 -> 0x760ad2428f897a994a0377ef5a3b626dfe295672
// x() -> 1
// f(uint256): 20 -> FAILURE
// x() -> 1
// stack(uint256): 1023 -> FAILURE
// gas irOptimized: 252410
// gas legacy: 477722
// gas legacyOptimized: 299567
// x() -> 1
// stack(uint256): 10 -> 0xd64ba06e96a2fa752f7c8e10c7b6912e518e40c2
// x() -> 2
