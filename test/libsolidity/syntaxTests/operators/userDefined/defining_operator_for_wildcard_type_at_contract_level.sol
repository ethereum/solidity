type Int is int;

function add(Int, Int) pure returns (Int) {}

contract C {
    using {add as +} for *;
}
// ----
// SyntaxError 3349: (81-104): The type has to be specified explicitly when attaching specific functions.
