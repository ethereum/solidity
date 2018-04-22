contract test {
	function f() pure public {
		bytes32 x = sha3(uint8(1));
		x;
	}
	function g() public {
		suicide(1);
	}
}
// ----
// Warning: (58-72): "sha3" has been deprecated in favour of "keccak256"
// Warning: (107-117): "suicide" has been deprecated in favour of "selfdestruct"
