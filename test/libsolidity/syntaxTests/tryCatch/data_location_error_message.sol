contract Foo {
    function test() public {
        try this.f() {}
        catch Error(string reason) {}
    }

    function f() public {
    }
}
// ----
// TypeError 6651: (88-101): Data location must be "memory" for parameter in function, but none was given.
