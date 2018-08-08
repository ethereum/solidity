contract C {
    modifier A { _; }
    apply A {
        function f() public pure {}
    }
}
