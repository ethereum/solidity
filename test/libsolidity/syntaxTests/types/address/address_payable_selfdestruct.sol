contract C {
    function f(address payable a) public {
        selfdestruct(a);
    }
}
