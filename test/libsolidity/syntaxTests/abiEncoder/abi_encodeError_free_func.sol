function g(uint) {}

function f() {
    abi.encodeError(g, (1));
}
// ----
// TypeError 3510: (56-57): Expected an error type. Cannot use functions for abi.encodeError.
