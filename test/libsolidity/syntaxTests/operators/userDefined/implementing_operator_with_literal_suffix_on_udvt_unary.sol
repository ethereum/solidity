type Int is int;
using {unsub as -} for Int global;

function unsub(Int x) pure suffix returns (Int) {
    return Int.wrap(-Int.unwrap(x));
}
