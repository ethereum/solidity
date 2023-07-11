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
    function testBitwise(I16 x, I16 y) public pure {
        assert(x | y == bitor(x, y)); // FIXME: should hold
        assert(x & y == bitand(x, y)); // FIXME: should hold
        assert(x ^ y == bitxor(x, y)); // FIXME: should hold
        assert(~x == bitnot(x)); // FIXME: should hold
    }

    function testArithmetic(I16 x, I16 y) public pure {
        assert(x + y == add(x, y)); // FIXME: should hold
        assert(x - y == sub(x, y)); // FIXME: should hold
        assert(-x == unsub(x)); // FIXME: should hold
        assert(x * y == mul(x, y)); // FIXME: should hold
        assert(x / y == div(x, y)); // FIXME: should hold
        assert(x % y == mod(x, y)); // FIXME: should hold
    }

    function testComparison(I16 x, I16 y) public pure {
        assert((x == y) == eq(x, y)); // FIXME: should hold
        assert((x != y) == noteq(x, y)); // FIXME: should hold
        assert((x < y) == lt(x, y)); // FIXME: should hold
        assert((x > y) == gt(x, y)); // FIXME: should hold
        assert((x <= y) == leq(x, y)); // FIXME: should hold
        assert((x >= y) == geq(x, y)); // FIXME: should hold
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6756: (1803-1808): User-defined operators are not yet supported by SMTChecker. This invocation of operator | has been ignored, which may lead to incorrect results.
// Warning 6756: (1803-1823): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (1863-1868): User-defined operators are not yet supported by SMTChecker. This invocation of operator & has been ignored, which may lead to incorrect results.
// Warning 6756: (1863-1884): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (1924-1929): User-defined operators are not yet supported by SMTChecker. This invocation of operator ^ has been ignored, which may lead to incorrect results.
// Warning 6756: (1924-1945): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6156: (1985-1987): User-defined operators are not yet supported by SMTChecker. This invocation of operator ~ has been ignored, which may lead to incorrect results.
// Warning 6756: (1985-2000): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2103-2108): User-defined operators are not yet supported by SMTChecker. This invocation of operator + has been ignored, which may lead to incorrect results.
// Warning 6756: (2103-2121): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2161-2166): User-defined operators are not yet supported by SMTChecker. This invocation of operator - has been ignored, which may lead to incorrect results.
// Warning 6756: (2161-2179): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6156: (2219-2221): User-defined operators are not yet supported by SMTChecker. This invocation of operator - has been ignored, which may lead to incorrect results.
// Warning 6756: (2219-2233): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2273-2278): User-defined operators are not yet supported by SMTChecker. This invocation of operator * has been ignored, which may lead to incorrect results.
// Warning 6756: (2273-2291): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2331-2336): User-defined operators are not yet supported by SMTChecker. This invocation of operator / has been ignored, which may lead to incorrect results.
// Warning 6756: (2331-2349): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2389-2394): User-defined operators are not yet supported by SMTChecker. This invocation of operator % has been ignored, which may lead to incorrect results.
// Warning 6756: (2389-2407): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2511-2517): User-defined operators are not yet supported by SMTChecker. This invocation of operator == has been ignored, which may lead to incorrect results.
// Warning 6756: (2571-2577): User-defined operators are not yet supported by SMTChecker. This invocation of operator != has been ignored, which may lead to incorrect results.
// Warning 6756: (2634-2639): User-defined operators are not yet supported by SMTChecker. This invocation of operator < has been ignored, which may lead to incorrect results.
// Warning 6756: (2693-2698): User-defined operators are not yet supported by SMTChecker. This invocation of operator > has been ignored, which may lead to incorrect results.
// Warning 6756: (2752-2758): User-defined operators are not yet supported by SMTChecker. This invocation of operator <= has been ignored, which may lead to incorrect results.
// Warning 6756: (2813-2819): User-defined operators are not yet supported by SMTChecker. This invocation of operator >= has been ignored, which may lead to incorrect results.
// Warning 3944: (679-708): CHC: Underflow (resulting value less than -32768) happens here.
// Warning 4984: (679-708): CHC: Overflow (resulting value larger than 32767) happens here.
// Warning 3944: (777-806): CHC: Underflow (resulting value less than -32768) happens here.
// Warning 4984: (777-806): CHC: Overflow (resulting value larger than 32767) happens here.
// Warning 3944: (953-982): CHC: Underflow (resulting value less than -32768) happens here.
// Warning 4984: (953-982): CHC: Overflow (resulting value larger than 32767) happens here.
// Warning 4281: (1051-1080): CHC: Division by zero happens here.
// Warning 4281: (1149-1178): CHC: Division by zero happens here.
// Warning 6328: (1796-1824): CHC: Assertion violation happens here.
// Warning 6328: (1856-1885): CHC: Assertion violation happens here.
// Warning 6328: (1917-1946): CHC: Assertion violation happens here.
// Warning 6328: (1978-2001): CHC: Assertion violation happens here.
// Warning 6328: (2096-2122): CHC: Assertion violation happens here.
// Warning 6328: (2154-2180): CHC: Assertion violation happens here.
// Warning 6328: (2212-2234): CHC: Assertion violation happens here.
// Warning 6328: (2266-2292): CHC: Assertion violation happens here.
// Warning 6328: (2324-2350): CHC: Assertion violation happens here.
// Warning 6328: (2382-2408): CHC: Assertion violation happens here.
// Warning 6328: (2503-2531): CHC: Assertion violation happens here.
// Warning 6328: (2563-2594): CHC: Assertion violation happens here.
// Warning 6328: (2626-2653): CHC: Assertion violation happens here.
// Warning 6328: (2685-2712): CHC: Assertion violation happens here.
// Warning 6328: (2744-2773): CHC: Assertion violation happens here.
// Warning 6328: (2805-2834): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
