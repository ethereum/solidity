contract A {
    function transfer() pure public {}
}

contract B {
    A a;

    function() external {
        a.transfer();
    }
}
