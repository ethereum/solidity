contract test {
	function intName() public {
		type(int).name;
	}
}
// ----
// TypeError: (47-61): Member "name" not found or not visible after argument-dependent lookup in type(int256).
