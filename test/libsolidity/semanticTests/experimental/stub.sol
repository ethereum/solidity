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

type uint256 = word;

instantiation uint256 : A {
    function testValue(x:uint256) -> y:word {
        assembly {
            y := 21
        }
    }
}

contract C {
	fallback() external {
		let x : word : A;
		let y;
        let z: (word, word);
        let w: uint256;
		assembly {
			x := 0x42
		}
		z = f(z);
		y = f(x);
		y = B.testValue(x);
		y = A.testValue(w);
		assembly {
			mstore(0, y)
			return(0, 32)
		}
	}
}
// ====
// compileViaYul: true
// ----
// () -> 0
