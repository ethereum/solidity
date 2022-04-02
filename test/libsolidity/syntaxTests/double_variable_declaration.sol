contract test {
	function f() pure public {
		uint256 x;
		x = 1;
		if (true) { uint256 x; x = 2; }
	}
}
// ----
// Warning 2519: (80-89='uint256 x'): This declaration shadows an existing declaration.
