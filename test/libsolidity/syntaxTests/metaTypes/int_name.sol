contract test {
	function intName() public {
		type(int).name;
	}
}
// ----
// TypeError 9582: (47-61='type(int).name'): Member "name" not found or not visible after argument-dependent lookup in type(int256).
