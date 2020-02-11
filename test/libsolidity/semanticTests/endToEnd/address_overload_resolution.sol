contract C {
    function balance() public returns(uint) {
        return 1;
    }

    function transfer(uint amount) public returns(uint) {
        return amount;
    }
}
contract D {
    function f() public returns(uint) {
        return (new C()).balance();
    }

    function g() public returns(uint) {
        return (new C()).transfer(5);
    }
}

// ----
f(): ""
g(): ""
