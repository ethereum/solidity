pragma experimental solidity;


class a:A {
    function testValue(x:a) -> y:word;
}


class a:B {
    function testValue(x:a) -> y:word;
}

instantiation word : A {
    function testValue(x:word) -> y:word {
        assembly {
            y := 7
        }
    }
}

instantiation word : B {
    function testValue(x:word) -> y:word {
        assembly {
            y := 14
        }
    }
}

function f(a:_:A) -> b:_:B {
    return a;
}

contract C {
	fallback() external {
		let x : word : A;
		let y;
		assembly {
			x := 0x42
		}
		y = f(x);
		y = B.testValue(x);
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
