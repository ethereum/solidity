pragma experimental solidity;

type word = __builtin("word");
type bool = __builtin("bool");
type integer = __builtin("integer");

type uint256 = word;

instantiation uint256: + {
    function add(x, y) -> uint256 {
        let a = uint256.rep(x);
        let b = uint256.rep(y);
        assembly {
            a := add(a,b)
        }
        return uint256.abs(a);
    }
}


instantiation uint256: * {
    function mul(x, y) -> uint256 {
        let a = uint256.rep(x);
        let b = uint256.rep(y);
        assembly {
            a := mul(a,b)
        }
        return uint256.abs(a);
    }
}
instantiation word: * {
    function mul(x, y) -> word {
        let z: word;
        assembly {
            z := mul(x,y)
        }
        return z;
    }
}

instantiation word: Integer {
    function fromInteger(x:integer) -> word {
        //x + x;
    }
}

instantiation word: == {
    function eq(x, y) -> bool {
        assembly {
            x := eq(x, y)
        }
    }
}


function f(x:uint256->uint256,y:uint256) -> uint256
{
    return x(y);
}

function g(x:uint256) -> uint256
{
    return x;
}

contract C {
    fallback() external {
        let arg;
        assembly {
            arg := calldataload(0)
        }
        let x : word;
        if (bool.abs(arg)) {
            assembly {
                x := 0x10
            }
        }
        let w = uint256.abs(x);
//        w = f(g, w);
        w = w * w + w;
        let y : word;
        let z : (uint256,uint256);
        assembly { y := 2 }
        y = uint256.rep(w) * y;
        assembly {
            mstore(0, y)
            return(0, 32)
        }
    }
}
// ====
// EVMVersion: >=constantinople
// ====
// compileViaYul: true
// ----
// (): 0 -> 0
// (): 1 -> 544
