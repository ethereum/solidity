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
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
