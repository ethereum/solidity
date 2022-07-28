type X is uint24;
type Y is uint16;

using {addX as +} for X;
using {addY as +} for Y;

function addX(X, X) pure returns (X) {}
function addY(Y, Y) pure returns (Y) { revert(); }

contract C {
    function f() public pure {
        X.wrap(1) + X.wrap(Y.unwrap(Y.wrap(2) + Y.wrap(3)));
        X.wrap(4) + X.wrap(5); // Unreachable
    }
}
// ----
// Warning 5740: (293-314): Unreachable code.
