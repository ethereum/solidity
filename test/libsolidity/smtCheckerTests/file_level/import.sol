==== Source: A ====
struct S { uint x; }
function set(S storage a, uint v) { a.x = v; }

==== Source: B ====
import "A";
import "A" as A;
contract C {
	A.S data;
	function f(uint v) internal returns (uint one, uint two) {
		A.set(data, v);
		one = data.x;
		set(data, v + 1);
		two = data.x;
	}
	function g() public {
		(uint x, uint y) = f(7);
		assert(x == 7); // should hold but the SMTChecker doesn't know that
		assert(y == 8); // should hold but the SMTChecker doesn't know that
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (B:238-252): CHC: Assertion violation happens here.
// Warning 6328: (B:308-322): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
