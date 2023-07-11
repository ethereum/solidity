contract C {

	modifier m() {
		unchecked{}
		_;
	}

	function t() m internal pure {}

	function f() public pure {
		unchecked { t(); }
	}
}
// ====
// SMTEngine: all
// ----
