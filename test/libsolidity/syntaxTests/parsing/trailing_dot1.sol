contract test {
	uint256 a = 2.2e10;
	uint256 b = .5E10;
	uint256 c = 4.e-2;
}
// ----
// TypeError: (70-73): Member "e" not found or not visible after argument-dependent lookup in int_const 4
