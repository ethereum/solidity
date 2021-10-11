==== Source: A ====
function id(uint x) pure returns (uint) {
    return x;
}
// we check that the effect of this directive is
// limited to this file.
using {id} for uint;

function t() pure {
    uint y = 2;
    y = y.id();
}

==== Source: B ====
import "A";

function f() pure {
    uint y = 2;
    y = y.id();
}
// ----
// TypeError 9582: (B:57-61): Member "id" not found or not visible after argument-dependent lookup in uint256.
