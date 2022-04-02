contract test {
    function f() public returns (uint ret_k, uint) {
        return 5;
    }
}
// ----
// TypeError 8863: (77-85='return 5'): Different number of arguments in return statement than in returns declaration.
