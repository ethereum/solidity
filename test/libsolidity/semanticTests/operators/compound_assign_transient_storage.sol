contract test {
	uint value1;
	uint transient value2;
	function f(uint x, uint y) public returns (uint w) {
		uint value3 = y;
		value1 += x;
		value3 *= x;
		value2 += value3 + value1;
		return value2 += 7;
	}
}
// ====
// EVMVersion: >=cancun
// ----
// f(uint256,uint256): 0, 6 -> 7
// f(uint256,uint256): 1, 3 -> 11
// f(uint256,uint256): 2, 25 -> 0x3c
// f(uint256,uint256): 3, 69 -> 0xdc
// f(uint256,uint256): 4, 84 -> 353
// f(uint256,uint256): 5, 2 -> 0x20
// f(uint256,uint256): 6, 51 -> 334
// f(uint256,uint256): 7, 48 -> 371
