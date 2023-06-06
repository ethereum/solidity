// fallback---------->f----------+
//                    |          |
//                    |          |
//                    |          |
//                    |          |
//                    v          v
//                    g          h
//
//
// add
//
// unreferenced

pragma experimental solidity;

type uint256 = __builtin("word");

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

function unreferenced(x:uint256) -> uint256
{
    return x;
}

function f(x:uint256) -> uint256
{
    return g(h(x));
}

function g(x:uint256) -> uint256
{
    return x;
}

function h(x:uint256) -> uint256
{
    return x;
}

contract C {
    fallback() external {
        let a: uint256->uint256 = f;
    }
}
// ----
// (add) --> {}
// (unreferenced) --> {}
// (f) --> {g,h,}
// (g) --> {}
// (h) --> {}
// (fallback) --> {f,}
