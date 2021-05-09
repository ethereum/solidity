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
