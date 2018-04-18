contract C {
    function (uint) external public x;

    function g(uint) public {
        x = this.g;
    }
    function f() public view returns (function(uint) external) {
        return this.x();
    }
}
// ----
