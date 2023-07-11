type I16 is int16;
using {
    bitor as |, bitand as &, bitxor as ^, bitnot as ~,
    add as +, sub as -, unsub as -, mul as *, div as /, mod as %,
    eq as ==, noteq as !=, lt as <, gt as >, leq as <=, geq as >=
} for I16 global;

function bitor(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) | I16.unwrap(y)); }
function bitand(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) & I16.unwrap(y)); }
function bitxor(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) ^ I16.unwrap(y)); }
function bitnot(I16 x) pure returns (I16) { return I16.wrap(~I16.unwrap(x)); }

function add(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) + I16.unwrap(y)); }
function sub(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) - I16.unwrap(y)); }
function unsub(I16 x) pure returns (I16) { return I16.wrap(-I16.unwrap(x)); }
function mul(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) * I16.unwrap(y)); }
function div(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) / I16.unwrap(y)); }
function mod(I16 x, I16 y) pure returns (I16) { return I16.wrap(I16.unwrap(x) % I16.unwrap(y)); }

function eq(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) == I16.unwrap(y); }
function noteq(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) != I16.unwrap(y); }
function lt(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) < I16.unwrap(y); }
function gt(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) > I16.unwrap(y); }
function leq(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) <= I16.unwrap(y); }
function geq(I16 x, I16 y) pure returns (bool) { return I16.unwrap(x) >= I16.unwrap(y); }

contract C {
    I16 constant MINUS_TWO = I16.wrap(-2);
    I16 constant ZERO = I16.wrap(0);
    I16 constant ONE = I16.wrap(1);
    I16 constant TWO = I16.wrap(2);
    I16 constant THREE = I16.wrap(3);
    I16 constant FOUR = I16.wrap(4);

    function testBitwise() public pure {
        assert(ONE | TWO == THREE); // FIXME: should hold
        assert(ONE & THREE == ZERO); // FIXME: should hold
        assert(TWO ^ TWO == ZERO); // FIXME: should hold
        assert(~ONE == MINUS_TWO); // FIXME: should hold
    }

    function testArithmetic() public pure {
        assert(TWO + TWO == FOUR); // FIXME: should hold
        assert(TWO - TWO == ZERO); // FIXME: should hold
        assert(-TWO == MINUS_TWO); // FIXME: should hold
        assert(TWO * TWO == FOUR); // FIXME: should hold
        assert(TWO / TWO == ONE); // FIXME: should hold
        assert(TWO % TWO == ZERO); // FIXME: should hold
    }

    function testComparison() public pure {
        assert(TWO == TWO); // FIXME: should hold
        assert(!(TWO != TWO)); // FIXME: should hold
        assert(!(TWO < TWO)); // FIXME: should hold
        assert(!(TWO > TWO)); // FIXME: should hold
        assert(TWO <= TWO); // FIXME: should hold
        assert(TWO >= TWO); // FIXME: should hold
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6756: (2019-2028): User-defined operators are not yet supported by SMTChecker. This invocation of operator | has been ignored, which may lead to incorrect results.
// Warning 6756: (2019-2037): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2077-2088): User-defined operators are not yet supported by SMTChecker. This invocation of operator & has been ignored, which may lead to incorrect results.
// Warning 6756: (2077-2096): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2136-2145): User-defined operators are not yet supported by SMTChecker. This invocation of operator ^ has been ignored, which may lead to incorrect results.
// Warning 6756: (2136-2153): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6156: (2193-2197): User-defined operators are not yet supported by SMTChecker. This invocation of operator ~ has been ignored, which may lead to incorrect results.
// Warning 6756: (2193-2210): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2301-2310): User-defined operators are not yet supported by SMTChecker. This invocation of operator + has been ignored, which may lead to incorrect results.
// Warning 6756: (2301-2318): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2358-2367): User-defined operators are not yet supported by SMTChecker. This invocation of operator - has been ignored, which may lead to incorrect results.
// Warning 6756: (2358-2375): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6156: (2415-2419): User-defined operators are not yet supported by SMTChecker. This invocation of operator - has been ignored, which may lead to incorrect results.
// Warning 6756: (2415-2432): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2472-2481): User-defined operators are not yet supported by SMTChecker. This invocation of operator * has been ignored, which may lead to incorrect results.
// Warning 6756: (2472-2489): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2529-2538): User-defined operators are not yet supported by SMTChecker. This invocation of operator / has been ignored, which may lead to incorrect results.
// Warning 6756: (2529-2545): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2585-2594): User-defined operators are not yet supported by SMTChecker. This invocation of operator % has been ignored, which may lead to incorrect results.
// Warning 6756: (2585-2602): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2693-2703): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2745-2755): User-defined operators are not yet supported by SMTChecker. This invocation of operator != has been ignored, which may lead to incorrect results.
// Warning 6756: (2798-2807): User-defined operators are not yet supported by SMTChecker. This invocation of operator < has been ignored, which may lead to incorrect results.
// Warning 6756: (2850-2859): User-defined operators are not yet supported by SMTChecker. This invocation of operator > has been ignored, which may lead to incorrect results.
// Warning 6756: (2900-2910): User-defined operators are not yet supported by SMTChecker. This invocation of operator <= has been ignored, which may lead to incorrect results.
// Warning 6756: (2950-2960): User-defined operators are not yet supported by SMTChecker. This invocation of operator >= has been ignored, which may lead to incorrect results.
// Warning 6328: (2012-2038): CHC: Assertion violation happens here.
// Warning 6328: (2070-2097): CHC: Assertion violation happens here.
// Warning 6328: (2129-2154): CHC: Assertion violation happens here.
// Warning 6328: (2186-2211): CHC: Assertion violation happens here.
// Warning 6328: (2294-2319): CHC: Assertion violation happens here.
// Warning 6328: (2351-2376): CHC: Assertion violation happens here.
// Warning 6328: (2408-2433): CHC: Assertion violation happens here.
// Warning 6328: (2465-2490): CHC: Assertion violation happens here.
// Warning 6328: (2522-2546): CHC: Assertion violation happens here.
// Warning 6328: (2578-2603): CHC: Assertion violation happens here.
// Warning 6328: (2686-2704): CHC: Assertion violation happens here.
// Warning 6328: (2736-2757): CHC: Assertion violation happens here.
// Warning 6328: (2789-2809): CHC: Assertion violation happens here.
// Warning 6328: (2841-2861): CHC: Assertion violation happens here.
// Warning 6328: (2893-2911): CHC: Assertion violation happens here.
// Warning 6328: (2943-2961): CHC: Assertion violation happens here.
