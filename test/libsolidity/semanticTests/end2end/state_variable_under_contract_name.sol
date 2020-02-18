
		contract Scope {
			uint stateVar = 42;

			function getStateVar() public view returns (uint stateVar) {
				stateVar = Scope.stateVar;
			}
		}
	
// ----
// getStateVar() -> 0x2a

