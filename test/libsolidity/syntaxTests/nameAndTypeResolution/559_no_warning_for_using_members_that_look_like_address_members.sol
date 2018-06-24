pragma experimental "v0.5.0";
contract C {
    function transfer(uint) public;
    function f() public {
        this.transfer(10);
    }
}
