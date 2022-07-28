type X is uint24;
type Y is uint16;

using {unsubX as -} for X;
using {unsubY as -} for Y;

function unsubX(X) pure returns (X) {}
function unsubY(Y) pure returns (Y) { revert(); }

contract C {
    function f() public pure {
        -X.wrap(Y.unwrap(-Y.wrap(1)));
        -X.wrap(Y.unwrap(Y.wrap(2))); // Unreachable
    }
}
// ----
// Warning 5740: (273-301): Unreachable code.
