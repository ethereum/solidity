contract TransferTest {
	fallback() external payable {
		// This used to cause an ICE
		payable(this).transfer;
	}

	function f() pure public {}
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() ->
