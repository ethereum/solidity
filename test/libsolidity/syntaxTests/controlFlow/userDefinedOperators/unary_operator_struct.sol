struct S { Z z; }
struct Z { int x; }

using {unsubS as -} for S;
using {unsubZ as -} for Z;

function unsubS(S memory) pure returns (S memory) {}
function unsubZ(Z memory) pure returns (Z memory) { revert(); }

contract C {
    function f() public pure {
        -S(-Z(1));
        -S(Z(2)); // Unreachable
    }
}
// ----
// Warning 5740: (283-291): Unreachable code.
