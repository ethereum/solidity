contract A {}

contract B is A {}

contract C {
	A[10] a;
	B[10] b;

	function f() public returns(address) {
		a = b;
		return address(a[0]);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
