error f(uint, uint);
contract C {
    function f(uint) public {
        revert f(10);
    }
}
// ----
// Warning 2519: (38-91): This declaration shadows an existing declaration.
// TypeError 1885: (79-80): Expression has to be an error.
