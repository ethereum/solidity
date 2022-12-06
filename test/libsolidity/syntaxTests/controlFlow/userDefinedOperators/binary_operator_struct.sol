struct S { Z z; }
struct Z { int x; }

using {addS as +} for S;
using {addZ as +} for Z;

function addS(S memory, S memory) pure returns (S memory) {}
function addZ(Z memory, Z memory) pure returns (Z memory) { revert(); }

contract C {
    function f() public pure {
        S(Z(1)) + S(Z(2) + Z(3));
        S(Z(4)) + S(Z(5)); // Unreachable
    }
}
// ----
// Warning 5740: (310-327): Unreachable code.
