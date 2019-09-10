pragma experimental ABIEncoderV2;

contract C {
	struct S2 { uint256 b; }
	struct S { uint256 a; S2[] children; }
	function f(S calldata s) external pure returns (uint256, uint256, uint256, uint256) {
		return (s.children.length, s.a, s.children[0].b, s.children[1].b);
	}
}
// ----
// f((uint256,(uint256)[])): 32, 17, 64, 2, 23, 42 -> 2, 17, 23, 42
