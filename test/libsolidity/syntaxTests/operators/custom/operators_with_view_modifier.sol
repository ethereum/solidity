type Int is uint8;

using {
    add as +,
    sub as -,
    mul as *
} for Int;


function f_view() view {}
function f() {}

function add(Int, Int) view returns (Int) {
    f_view();
    return Int.wrap(0);
}

function sub(Int, Int) returns (Int) {
    f_view();
    return Int.wrap(0);
}

function mul(Int, Int) view returns (Int) {
    f();
    return Int.wrap(0);
}

// ----
// Warning 2018: (210-288): Function state mutability can be restricted to view
// TypeError 8961: (338-341): Function cannot be declared as view because this expression (potentially) modifies the state.
