contract test {
	enum MinMax { A, B, C, D }

	function min() public returns(uint) { return uint(type(MinMax).min); }
	function max() public returns(uint) { return uint(type(MinMax).max); }
}

// ----
// min() -> 0
// max() -> 3
