error E(uint);

function f() pure {
    abi.encodeError(E, (1));
}
// ----
