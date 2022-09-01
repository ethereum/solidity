error E(uint);

function f() {
    abi.encodeCall(E, (1));
}
// ----
// TypeError 3509: (50-51): Expected regular external function type, or external view on public function. Cannot use errors for abi.encodeCall.
