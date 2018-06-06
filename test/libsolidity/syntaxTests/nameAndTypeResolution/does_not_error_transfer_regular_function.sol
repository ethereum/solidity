contract A {
    function transfer() pure public {}
}

contract B {
    A a;

    function() public {
        a.transfer();
    }
}
