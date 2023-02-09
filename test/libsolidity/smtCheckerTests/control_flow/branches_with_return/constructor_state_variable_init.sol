contract A {
	int x;
	constructor (int a) { x = a;}
}

contract B is A {
	int y;
	constructor(int a) A(-a) {
		if (a > 0) {
			y = 2;
			return;
		}
		else {
			y = 3;
		}
		y = 4; // overwrites the else branch
	}
}

contract C is B {
	constructor(int a) B(a) {
		assert(y != 3); // should hold
		assert(y == 4); // should fail
		if (a > 0) {
			assert(x < 0 && y == 2); // should hold
			assert(x < 0 && y == 4); // should fail
		}
		else {
			assert(x >= 0 && y == 4); // should hold
			assert(x >= 0 && y == 2); // should fail
			assert(x > 0); // should fail
		}
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (297-311): CHC: Assertion violation happens here.
// Warning 6328: (389-412): CHC: Assertion violation happens here.
// Warning 6328: (489-513): CHC: Assertion violation happens here.
// Warning 6328: (533-546): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
