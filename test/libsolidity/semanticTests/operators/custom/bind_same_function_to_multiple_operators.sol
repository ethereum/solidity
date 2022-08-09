type Int is int32;

using {foo as +, foo as -} for Int;

function foo(Int, Int) pure returns(Int) {
    return Int.wrap(7);
}

contract C {
    function f() pure public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }

    function g() pure public returns (Int) {
        return Int.wrap(0) - Int.wrap(0);
    }
}

// ----
// f() -> 7
// g() -> 7
