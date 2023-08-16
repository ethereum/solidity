contract C {
	function f() public {}
	function g() public {}
	function h(bool c) public returns (bytes4) {
		return (c ? this.f : this.g).selector;
	}
}
// ----
// h(bool): true -> 0x26121ff000000000000000000000000000000000000000000000000000000000
// h(bool): false -> 0xe2179b8e00000000000000000000000000000000000000000000000000000000
