contract C {
    uint public x;
    function f() payable public {
    }
    function g() pure public returns (bytes4) {
        return this.f.selector ^ this.x.selector;
    }
}
