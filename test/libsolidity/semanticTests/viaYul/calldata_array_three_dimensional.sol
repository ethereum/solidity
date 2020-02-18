pragma experimental ABIEncoderV2;

contract C {
	function f(uint256[][2][] calldata x, uint256 i, uint256 j, uint256 k) external returns (uint256 a, uint256 b, uint256 c, uint256 d) {
		a = x.length;
		b = x[i].length;
		c = x[i][j].length;
		d = x[i][j][k];
    }
}
// ====
// compileViaYul: also
// ----
// f(uint256[][2][],uint256,uint256,uint256): 0x80, 0, 0, 0, 1, 0x20, 0x40, 0x80, 1, 42, 1, 23 -> 1, 2, 1, 42
// f(uint256[][2][],uint256,uint256,uint256): 0x80, 0, 1, 0, 1, 0x20, 0x40, 0x80, 1, 42, 1, 23 -> 1, 2, 1, 23
// f(uint256[][2][],uint256,uint256,uint256): 0x80, 0, 1, 0, 1, 0x20, 0x40, 0x80, 1, 42, 2, 23, 17 -> 1, 2, 2, 23
// f(uint256[][2][],uint256,uint256,uint256): 0x80, 0, 1, 1, 1, 0x20, 0x40, 0x80, 1, 42, 2, 23, 17 -> 1, 2, 2, 17
// f(uint256[][2][],uint256,uint256,uint256): 0x80, 1, 0, 0, 1, 0x20, 0x40, 0x80, 1, 42, 1, 23 -> FAILURE
// f(uint256[][2][],uint256,uint256,uint256): 0x80, 0, 2, 0, 1, 0x20, 0x40, 0x80, 1, 42, 1, 23 -> FAILURE
// f(uint256[][2][],uint256,uint256,uint256): 0x80, 0, 2, 0, 1, 0x20, 0x40, 0x80, 1, 42, 1, 23 -> FAILURE
