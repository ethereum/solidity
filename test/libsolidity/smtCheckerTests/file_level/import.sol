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
// ----
// Warning 8364: (B:115-116): Assertion checker does not yet implement type module "A"
// Warning 6328: (B:238-252): CHC: Assertion violation happens here.\nCounterexample:\ndata = {x: 0}\nx = 0\ny = 0\n\nTransaction trace:\nC.constructor()\nState: data = {x: 0}\nC.g()\n    C.f(7) -- internal call\n        A:set({x: 0}, 7) -- internal call\n        A:set({x: 0}, 8) -- internal call
// Warning 6328: (B:308-322): CHC: Assertion violation happens here.\nCounterexample:\ndata = {x: 0}\nx = 0\ny = 0\n\nTransaction trace:\nC.constructor()\nState: data = {x: 0}\nC.g()\n    C.f(7) -- internal call\n        A:set({x: 0}, 7) -- internal call\n        A:set({x: 0}, 8) -- internal call
// Warning 8364: (B:115-116): Assertion checker does not yet implement type module "A"
