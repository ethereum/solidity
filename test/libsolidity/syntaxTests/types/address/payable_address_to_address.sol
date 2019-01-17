contract C {
    function f(address payable a) public pure {
        address payable b;
        address c = a;
        c = b;
    }
}