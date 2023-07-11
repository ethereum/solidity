contract C {
	struct S {
		int[][] d;
	}
	S[] data;
	constructor() {
		data.push();
		data.push();
		data[1].d.push();
		data[1].d.push();
		data[1].d.push();
		data[1].d.push();
	}
	function f() public {
		++data[1].d[3].push();
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 7 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
