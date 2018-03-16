contract C {
    function f() public {
        delete this.f;
    }
}
