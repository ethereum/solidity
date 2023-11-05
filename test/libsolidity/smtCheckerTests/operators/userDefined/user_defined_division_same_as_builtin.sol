type U is uint;
using {div as /} for U global;

function div(U x, U y) pure returns (U) {
    return U.wrap(U.unwrap(x) / U.unwrap(y)); // detects division by zero
}

contract C {
    function f(U x, U y) public pure returns (U) {
        return x / y; // reports division by zero in the implementation
    }
}
// ====
// SMTEngine: all
// ----
// Warning 4281: (108-133): CHC: Division by zero happens here.
