contract C {
	function f() public {
		try this.f() {}
		catch (bytes memory x) {
			x;
		}
	}
}
// ====
// SMTEngine: all
// ----
// Warning 3993: The BMC engine of the SMTChecker is deprecated and will be removed in a future release. Please use the CHC engine instead.
