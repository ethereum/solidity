library L {
    event E(uint);
}

function f() {
    abi.encodeError(L.E, (1));
}
// ----
// TypeError 3510: (69-72): Expected an error type. Cannot use events for abi.encodeError.
