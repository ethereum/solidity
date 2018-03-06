pragma experimental "v0.5.0";
contract test {
	function f() pure public {
		uint256 x;
		if (true) { uint256 x; }
	}
}
// ----
// Warning: This declaration shadows an existing declaration.
// Warning: Unused local variable.
// Warning: Unused local variable.
