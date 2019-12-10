abstract contract C {
    function transfer(uint) public virtual;
    function f() public {
        this.transfer(10);
    }
}
// ----
