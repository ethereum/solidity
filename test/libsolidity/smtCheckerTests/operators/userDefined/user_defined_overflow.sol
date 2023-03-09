type U8 is uint8;
using {add as +} for U8 global;

function add(U8 x, U8 y) pure returns (U8) {
    return U8.wrap(U8.unwrap(x) + U8.unwrap(y)); // FIXME: should detect possible overflow here
}

contract C {
    U8 x = U8.wrap(254);

    function inc() public {
        x = x + U8.wrap(1); // FIXME: should detect possible overflow here
    }

    function check() view public {
        U8 y = x;
        assert(U8.unwrap(y) < 256);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6756: (274-288): User-defined operators are not yet supported by SMTChecker. This invocation of operator + has been ignored, which may lead to incorrect results.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
