type Int is int32;

using {foo as +, foo as -} for Int global;

function foo(Int a, Int b) pure returns(Int) {
    return Int.wrap(Int.unwrap(a) + Int.unwrap(b));
}

contract C {
    function f() pure public returns (Int) {
        return Int.wrap(2) + Int.wrap(3);
    }

    function g() pure public returns (Int) {
        return Int.wrap(6) - Int.wrap(1);
    }
}
// ----
// f() -> 5
// g() -> 7
