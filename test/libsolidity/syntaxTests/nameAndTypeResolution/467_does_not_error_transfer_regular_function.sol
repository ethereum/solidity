contract A {
    function transfer() pure public {}
}

contract B {
    A a;

    fallback() external {
        a.transfer();
    }
}
