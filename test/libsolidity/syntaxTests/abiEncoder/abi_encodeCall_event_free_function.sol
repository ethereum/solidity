library L {
    event E(uint);
}

function f() {
    abi.encodeCall(L.E, (1));
}
// ----
// TypeError 3509: (68-71): Expected regular external function type, or external view on public function. Cannot use events for abi.encodeCall.
