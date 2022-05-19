contract C {
	function f() public returns (bool) {
		return
			0 <
			~~84926290883049832306107864558384249403874903260938453235235091622489261765859;
	}
}
// ----
// f() -> true
