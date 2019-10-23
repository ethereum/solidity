contract C {
    function transfer(uint) public;
    function f() public {
        this.transfer(10);
    }
}
// ----
// TypeError: (0-109): Contract "C" should be marked as abstract.
