type Int is int16;

using {add as +, add} for Int;

function add(Int _a, Int _b) pure returns (Int) {
    return Int.wrap(Int.unwrap(_a) + Int.unwrap(_b));
}

contract C {
    function f() pure public returns (Int) {
        return Int.wrap(5) + Int.wrap(5);
    }

    function g() pure public returns (Int) {
        return Int.wrap(7).add(Int.wrap(6));
    }
}

// ----
// f() -> 10
// g() -> 13
