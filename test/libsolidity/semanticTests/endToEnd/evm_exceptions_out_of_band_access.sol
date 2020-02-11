contract A {
    uint[3] arr;
    bool public test = false;

    function getElement(uint i) public returns(uint) {
        return arr[i];
    }

    function testIt() public returns(bool) {
        uint i = this.getElement(5);
        test = true;
        return true;
    }
}

// ----
// test() -> false
// testIt() -> FAILURE
// test() -> false
