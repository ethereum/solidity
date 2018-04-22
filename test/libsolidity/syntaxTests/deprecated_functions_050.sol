pragma experimental "v0.5.0";
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
// TypeError: (88-102): "sha3" has been deprecated in favour of "keccak256"
// TypeError: (137-147): "suicide" has been deprecated in favour of "selfdestruct"
