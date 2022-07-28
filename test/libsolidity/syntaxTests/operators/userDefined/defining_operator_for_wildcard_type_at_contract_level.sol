type Int is int;

function add(Int, Int) returns (Int) {}

contract C {
    using {add as +} for *;
}
// ----
// SyntaxError 3349: (76-99): The type has to be specified explicitly when attaching specific functions.
