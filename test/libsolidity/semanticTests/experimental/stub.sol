pragma experimental solidity;

type uint256 = word;

instantiation uint256: * {
    function mul(x, y) -> z {
        let a = uint256.rep(x);
        let b = uint256.rep(y);
        assembly {
            a := mul(a,b)
        }
        return uint256.abs(a);
    }
}

instantiation uint256: + {
    function add(x, y) -> z {
        let a = uint256.rep(x);
        let b = uint256.rep(y);
        assembly {
            a := add(a,b)
        }
        return uint256.abs(a);
    }
}

instantiation word: * {
    function mul(x, y) -> z {
        assembly {
            z := mul(x,y)
        }
    }
}

instantiation word: integer {
    function fromInteger(x:integer) -> y:word {
    }
}

instantiation word: == {
    function eq(x, y) -> z:bool {
        assembly {
            x := eq(x, y)
        }
    }
}

contract C {
	fallback() external {
		let x : word;
		assembly {
			x := 0x10
		}
        let w = uint256.abs(x);
        w = w * w + w;
		let y : word;
        assembly { y := 2 }
        y = uint256.rep(w) * y;
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
