pragma abicoder               v2;

abstract contract C {
	struct S {
	uint256 x;
	string y;
	}
	function f(address x) external virtual returns (uint256, string memory);
}
contract D is C {
	mapping(address => S) public override f;
}
// ----
