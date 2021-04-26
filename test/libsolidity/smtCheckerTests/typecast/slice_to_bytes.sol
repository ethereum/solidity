contract C {
	function f(bytes calldata x) external pure {
		bytes(x[:18726387213]);
		bytes(x[18726387213:]);
		bytes(x[18726387213:111111111111111111]);
	}
}
// ====
// SMTEngine: all
// ----
