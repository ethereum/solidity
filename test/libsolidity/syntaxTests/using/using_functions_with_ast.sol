function f(uint) {}

contract C {
    using {f} for *;
}
// ----
// SyntaxError 3349: (38-54): The type has to be specified explicitly when attaching specific functions.
