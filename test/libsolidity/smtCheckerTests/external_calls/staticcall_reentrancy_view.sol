contract C {
	uint x;
	function s(uint _x) public view {
		x == _x;
	}
	function f(address a) public {
		(bool s, bytes memory data) = a.staticcall("");
		assert(x == 0); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2519: (106-112): This declaration shadows an existing declaration.
// Warning 2072: (106-112): Unused local variable.
// Warning 2072: (114-131): Unused local variable.
// Warning 2018: (72-188): Function state mutability can be restricted to view
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
