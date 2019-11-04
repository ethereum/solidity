abstract contract C {
    function transfer(uint) public;
    function f() public {
        this.transfer(10);
    }
}
// ----
