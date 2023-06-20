pragma experimental solidity;


class a:StackType {
	function stackSize() -> x:integer;
}

instantiation word : StackType {
	function stackSize() -> x:integer {
		return x;
	}
}

function f(a) -> b {
    return a;
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
