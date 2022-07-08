type Int is int128;
using {
    bitnot as ~
} for Int;

function bitnot(Int, Int) pure returns (Int) {
    return Int.wrap(13);
}

contract C {
    function test() public pure {
        ~Int.wrap(1);
    }
}

// ----
// TypeError 1147: (32-38): The function "bitnot" needs to have exactly one parameter to be used for the operator ~.
// TypeError 4907: (186-198): Unary operator ~ cannot be applied to type Int
