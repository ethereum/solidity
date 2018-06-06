contract C {
    function f() pure external returns (bytes4) {
        return this.f.selector;
    }
}
