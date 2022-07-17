library L {
    function g() external {}
}

function f() {
    abi.encodeCall(L.g, (1));
}
// ----
// TypeError 3509: (78-81): Expected regular external function type, or external view on public function. Cannot use library functions for abi.encodeCall.
