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
		D d = new D{salt: bytes32(uint256(p))}(p);
		return d.getX();
	}
}
// ====
// EVMVersion: >=constantinople
// targetContract: A
// ----
// g(int256): -1 -> -1
// gas legacy: 77955
// gas legacy code: 24200
// g(int256): 10 -> 10
// gas legacy: 77583
// gas legacy code: 24200
