contract C {
	uint256[] x;
	constructor() { x.push(42); }
	function f() public {
		x.push(23);
		assert(x[0] == 42 || x[0] == 23);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 0: (0-135): Contract invariants for :C:\n(!((x[x.length] := 23)[0] <= 41) && !((x[x.length] := 23)[0] >= 43))\n
