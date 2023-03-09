contract C {

	struct S {
		uint x;
		uint y;
		uint z;
	}

	function test() pure public {
		S memory s = S({z: 1, y: 2, x: 3});
		assert(s.x == 3);
		assert(s.y == 2);
		assert(s.z == 1);
		assert(s.x == 0 || s.y == 0 || s.z == 0); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (191-231): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
