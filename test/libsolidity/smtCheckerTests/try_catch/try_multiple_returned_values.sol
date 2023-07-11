abstract contract D {
	function d() external virtual returns (uint x, bool b);
}

contract C {

	int x;
	D d;

	function f() public {
		x = 0;
		try d.d() returns (uint x, bool c) {
			assert(x == 0); // should fail, x is the local variable shadowing the state variable
			assert(!c); // should fail, c can be anything
		} catch {
			assert(x == 0); // should hold, x is the state variable
			assert(x == 1); // should fail
		}
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2519: (164-170): This declaration shadows an existing declaration.
// Warning 6328: (185-199): CHC: Assertion violation happens here.
// Warning 6328: (273-283): CHC: Assertion violation happens here.
// Warning 6328: (393-407): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
