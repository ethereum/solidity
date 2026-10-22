// Used to cause ICE in ABIFunctions::abiDecodingFunctionArrayAvailableLength
// when a dynamic array's element type was a zero-length static array.
contract C {
	function f(bytes memory b) public pure {
		abi.decode(b, (uint256[0][]));
	}
}
// ----
// TypeError 1406: (146-147): Array with zero length specified.
