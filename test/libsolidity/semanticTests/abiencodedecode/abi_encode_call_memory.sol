pragma abicoder v2;

contract D {
	function something() external pure {}
}

contract C {
	function something() external pure {}
	function test() external returns (bytes4) {
		function() external[2] memory x;
		x[0] = this.something;
		x[1] = (new D()).something;
		function() external f = x[1];
		bytes memory a = abi.encodeCall(x[0], ());
		bytes memory b = abi.encodeCall(x[1], ());
		bytes memory c = abi.encodeCall(f, ());
		assert(a.length == 4 && b.length == 4 && c.length == 4);
		assert(bytes4(a) == bytes4(b));
		assert(bytes4(a) == bytes4(c));
		assert(bytes4(a) == f.selector);
		return bytes4(a);
	}
}
// ----
// test() -> 0xa7a0d53700000000000000000000000000000000000000000000000000000000
