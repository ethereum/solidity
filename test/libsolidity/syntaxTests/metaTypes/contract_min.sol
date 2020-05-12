contract Min {
	function contractMin() public {
		type(Min).min;
	}
}
// ----
// TypeError: (50-63): Member "min" not found or not visible after argument-dependent lookup in type(contract Min).
