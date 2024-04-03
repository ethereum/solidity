library L {
    error g(uint);
}

function f() pure {
    abi.encodeError(L.g, (1));
}
// ----
