type Int is int;

function add(Int, Int) pure returns (Int) {}

contract C {
    using {add as +} for *;
}

contract D {
    using {add as +} for * global;
}
// ----
// SyntaxError 3349: (81-104): The type has to be specified explicitly when attaching specific functions.
// SyntaxError 3349: (125-155): The type has to be specified explicitly when attaching specific functions.
// SyntaxError 2854: (125-155): Can only globally attach functions to specific types.
// SyntaxError 3367: (125-155): "global" can only be used at file level.
