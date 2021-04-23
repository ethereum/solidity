contract C {
	function f() public payable returns (uint) {
		return msg.value;
	}
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(), 1 ether -> 1000000000000000000
// f(), 1 wei -> 1
