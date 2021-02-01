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
// gas ir: 168491
// gas legacy: 121455
// gas legacyOptimized: 112189
// g(int256): 10 -> 10
// gas ir: 168119
// gas legacy: 121083
// gas legacyOptimized: 111817
