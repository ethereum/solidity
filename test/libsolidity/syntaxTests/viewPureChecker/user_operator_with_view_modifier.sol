type Int is uint8;

using {
    add as +
} for Int;


function f_view() view {}

function add(Int, Int) view returns (Int) {
    f_view();
    return Int.wrap(0);
}

function f() view {
    Int.wrap(0) + Int.wrap(0);
}

// ----
