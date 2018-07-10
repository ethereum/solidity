pragma experimental "v0.5.0";
contract C {
	function f() public pure returns (uint, uint, bytes32) {
		uint a;
		bytes32 b;
		(a,) = f();
		(,b) = f();
	}
}
// ----
// TypeError: (133-136): Type tuple(uint256,uint256,bytes32) is not implicitly convertible to expected type tuple(uint256,).
// TypeError: (147-150): Type tuple(uint256,uint256,bytes32) is not implicitly convertible to expected type tuple(,bytes32).
