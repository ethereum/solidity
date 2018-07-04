contract C {
    uint public x;
    function f() payable public {
    }
    function g() pure public returns (bytes4) {
        return this.f.selector ^ this.x.selector;
    }
    function h() view public returns (bytes4) {
        x;
        return this.f.selector ^ this.x.selector;
    }
}
