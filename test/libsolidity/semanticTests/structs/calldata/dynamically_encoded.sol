pragma experimental ABIEncoderV2;

contract C {
	struct S { uint256[] a; }
	function f(S calldata s) external pure returns (uint256 a, uint256 b, uint256 c) {
	    return (s.a.length, s.a[0], s.a[1]);
	}
}
// ----
// f((uint256[])): 32, 32, 2, 42, 23 -> 2, 42, 23
