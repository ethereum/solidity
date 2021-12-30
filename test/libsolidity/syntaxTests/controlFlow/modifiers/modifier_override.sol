contract A {
	function f() mod internal returns (uint[] storage) {
	}
	modifier mod() virtual {
		revert();
		_;
	}
}
contract B is A {
	modifier mod() override { _; }
	function g() public {
		f()[0] = 42;
	}
}
// ----
// Warning 5740: (65-69): Unreachable code.
// TypeError 3464: (49-63): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
