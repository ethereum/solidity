==== Source: A ====
contract C {
	int private x;
	constructor (int p) public { x = p; }
	function getX() public returns (int) { return x; }
}
==== Source: B ====
import "A" as M;

contract D is M.C {
	constructor (int p) M.C(p) public {}
}

contract A {
	function g(int p) public returns (int) {
		D d = new D(p);
		return d.getX();
	}
}

// ====
// compileViaYul: also
// ----
// g(int256): -1 -> -1
// gas legacy: 103622
// g(int256): 10 -> 10
// gas legacy: 103250
