type U is uint;
using {div as /} for U global;

function div(U x, U y) pure returns (U) {
    if (U.unwrap(y) == 0)
        return U.wrap(0);

    return U.wrap(U.unwrap(x) / U.unwrap(y));
}

contract C {
    function f(U x, U y) public pure returns (U) {
        return x / y; // no div by zero possible here
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6756: (271-276): User-defined operators are not yet supported by SMTChecker. This invocation of operator / has been ignored, which may lead to incorrect results.
