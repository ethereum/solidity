error g(uint);

function f() pure {
    abi.encodeError(g, (1));
}
// ----
