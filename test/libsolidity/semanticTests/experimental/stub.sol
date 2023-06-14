pragma experimental solidity;

contract C {
	fallback() external {
	    word x;
		assembly {
			mstore(0, 42)
			return(0, 32)
		}
	}
}
// ====
// compileViaYul: true
// ----
// () -> 42
