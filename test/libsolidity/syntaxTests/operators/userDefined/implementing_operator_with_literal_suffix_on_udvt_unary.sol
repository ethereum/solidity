type Int is int;
using {unsub as -} for Int global;

function unsub(Int x) pure suffix returns (Int) {
    return Int.wrap(-Int.unwrap(x));
}
// ----
// TypeError 2998: (68-73): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
