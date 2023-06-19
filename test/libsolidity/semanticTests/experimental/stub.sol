pragma experimental solidity;

function f(a:word) -> (b:word) {
    assembly {
        b := a
    }
}

contract C {
	fallback() external {
		let x : word;
		let y : word;
		assembly {
			x := 0x42
		}
		y = x;
		assembly {
			mstore(0, y)
			return(0, 32)
		}
	}
}
// ====
// compileViaYul: true
// ----
// () -> 0x42
