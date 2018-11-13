contract test {
	function f() pure public {
		uint256 x;
		x = 1;
		if (true) { uint256 x; x = 2; }
	}
}
// ----
// Warning: (80-89): This declaration shadows an existing declaration.
