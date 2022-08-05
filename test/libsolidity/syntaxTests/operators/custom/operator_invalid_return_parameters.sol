type Int is int256;

using {
    add as +,
    div as /,
    unsub as -,
    bitnot as ~,
    gt as >,
    lt as <
} for Int;

function add(Int x, Int y) pure returns (int256) {
    return 0;
}

function div(Int x, Int y) pure {
    x = y;
}

function unsub(Int) pure returns (Int, Int) {
    return (Int.wrap(0), Int.wrap(1));
}

function bitnot(Int) pure returns (int256) {
    return 0;
}

function gt(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}

function lt(Int, Int) pure returns (bool, Int) {
    return (true, Int.wrap(1));
}

function f() pure {
    Int.wrap(0) + Int.wrap(1);
    Int.wrap(0) / Int.wrap(0);
    -Int.wrap(0);
    ~Int.wrap(0);
    Int.wrap(0) < Int.wrap(0);
    Int.wrap(0) > Int.wrap(0);
}

// ----
// TypeError 7743: (33-36): The function "add" needs to return exactly one value of type Int to be used for the operator +.
// TypeError 7743: (47-50): The function "div" needs to return exactly one value of type Int to be used for the operator /.
// TypeError 7743: (61-66): The function "unsub" needs to return exactly one value of type Int to be used for the operator -.
// TypeError 7743: (77-83): The function "bitnot" needs to return exactly one value of type Int to be used for the operator ~.
// TypeError 7995: (94-96): The function "gt" needs to return exactly one value of type bool to be used for the operator >.
// TypeError 7995: (107-109): The function "lt" needs to return exactly one value of type bool to be used for the operator <.
