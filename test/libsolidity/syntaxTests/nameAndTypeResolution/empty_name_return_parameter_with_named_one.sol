contract test {
    function f() public returns (uint ret_k, uint) {
        return 5;
    }
}
// ----
// TypeError: (77-85): Different number of arguments in return statement than in returns declaration.
