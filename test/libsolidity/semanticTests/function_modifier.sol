
		contract C {
			function getOne() payable nonFree public returns (uint r) { return 1; }
			modifier nonFree { if (msg.value > 0) _; }
		}
	
// ----
// getOne() -> 0
// getOne(), 1 wei -> 1

