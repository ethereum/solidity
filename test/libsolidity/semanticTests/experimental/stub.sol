pragma experimental solidity;

function f(a) -> (b) {
    b = a;
}

contract C {
	fallback() external {
		let x : word;
		let y : word;
		assembly {
			x := 0x42
		}
		y = f(x);
		assembly {
			mstore(0, y)
			return(0, 32)
		}
	}
}
// ====
// compileViaYul: true
// ----
// () -> 21
