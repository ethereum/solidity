type Int is int;
using {add as +, unsub as -} for Int global;

function add(Int x, Int y) pure suffix returns (Int) {
    return Int.wrap(Int.unwrap(x) + Int.unwrap(y));
}

function unsub(Int x) pure suffix returns (Int) {
    return Int.wrap(-Int.unwrap(x));
}
