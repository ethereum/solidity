contract test {
    function fun() public {
        address(0).balance = 7;
    }
}
// ----
// TypeError 4247: (52-70): Expression has to be an lvalue.
