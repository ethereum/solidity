contract C {
        function f() payable public returns (C) {
        return this;
    }
    function g() pure public returns (bytes4) {
        C x = C(0x123);
        return x.f.selector;
    }
}
