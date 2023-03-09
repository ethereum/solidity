contract A {
    int[] a;
    function f() public {
		A.a.push();
        A.a[0] = 2;
    }
}

// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
