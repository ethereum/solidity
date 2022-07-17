function g(uint) {}

function f() {
    abi.encodeCall(g, (1));
}
// ----
// TypeError 3509: (55-56): Expected regular external function type, or external view on public function. Provided internal function.
