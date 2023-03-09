abstract contract D {
	function d() external virtual;
}

contract C {

	int x;
	D d;

	function set(int n) public {
		x = n;
	}

	function f() public {
		x = 0;
		try d.d() {
			//assert(x == 0); // should fail, x can be anything here
		} catch {
			assert(x == 0); // should hold, all changes to x has been reverted
			assert(x == 1); // should fail
		}
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (320-334): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
