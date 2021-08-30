contract C {
	function g(address payable i) public {
		require(address(this).balance == 100);
		i.call{value: 10}("");
		// Disabled due to Spacer nondeterminism
		//assert(address(this).balance == 90); // should hold
		// Disabled due to Spacer nondeterminism
		//assert(address(this).balance == 100); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 9302: (96-117): Return value of low-level calls not used.
