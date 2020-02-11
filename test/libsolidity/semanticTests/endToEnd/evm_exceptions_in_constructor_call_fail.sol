contract A {
    constructor() public {
        address(this).call("123");
    }
}
contract B {
    uint public test = 1;

    function testIt() public {
        A a = new A();
        ++test;
    }
}

// ----
// testIt() -> 
// testIt():"" -> ""
// test() -> 2
// test():"" -> "2"
