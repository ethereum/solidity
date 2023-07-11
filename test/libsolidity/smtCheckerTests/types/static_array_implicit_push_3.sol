contract D {
	bytes16[] inner;
	bytes32[][] data;
	function t() public {
		data.push(inner);
	}
}
// ====
// SMTEngine: all
// ----
