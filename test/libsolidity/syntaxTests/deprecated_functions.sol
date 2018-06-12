contract test {
	function f() pure public {
		bytes32 x = sha3();
		x;
	}
	function g() public {
		suicide(1);
	}
}
// ----
// TypeError: (58-64): "sha3" has been deprecated in favour of "keccak256"
// TypeError: (99-109): "suicide" has been deprecated in favour of "selfdestruct"
