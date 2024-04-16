contract C {
	function zeroArgs() public pure returns(bytes memory) {
		bytes memory a = bytes.concat();
		assert(a.length == 0); // zero args call is an empty bytes array
		return bytes.concat();
	}

	function oneArg(bytes memory a) public pure returns(bytes memory) {
		return bytes.concat(a);
	}

	function oneArgFixedBytes(bytes8 a) public pure returns(bytes memory) {
		return bytes.concat(a);
	}

	function fixedBytes(bytes8 a, bytes8 b) public pure returns(bytes memory) {
		return bytes.concat(a, b);
	}

	function memoryBytes(bytes memory a, bytes memory b) public pure returns(bytes memory) {
		return bytes.concat(a, b);
	}

	function mixed(bytes memory a, bytes2 b) public pure returns(bytes memory) {
		return bytes.concat(a, b, "StringLiteral");
	}

	function functionCallAsArg(bytes memory a) public pure returns(bytes memory) {
		return bytes.concat(oneArg(a), zeroArgs());
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
