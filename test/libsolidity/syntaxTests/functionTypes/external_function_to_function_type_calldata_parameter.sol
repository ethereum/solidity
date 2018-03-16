contract C {
    function f(function(bytes memory) external g) public { }
    function callback(bytes) external {}
    function g() public {
        f(this.callback);
    }
}
