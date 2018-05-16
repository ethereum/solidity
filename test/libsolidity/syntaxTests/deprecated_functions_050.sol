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
// TypeError: (88-102): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// TypeError: (88-102): The provided argument of type uint8 is not implicitly convertible to expected type bytes memory.
// TypeError: (137-147): "suicide" has been deprecated in favour of "selfdestruct"
