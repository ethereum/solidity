type Int is int;

function add(Int, Int) returns (Int) {
    return Int.wrap(0);
}

contract C {
    using {add as +} for *;
}

// ----
// SyntaxError 3349: (101-124): The type has to be specified explicitly when attaching specific functions.
