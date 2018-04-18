pragma experimental "v0.5.0";
contract test {
	function f() pure public {
		uint256 x;
		if (true) { uint256 x; }
	}
}
// ----
// Warning: (101-110): This declaration shadows an existing declaration.
// Warning: (76-85): Unused local variable.
// Warning: (101-110): Unused local variable.
