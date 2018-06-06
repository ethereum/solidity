contract A {
    enum Foo {
        First,
        Second
    }

    function a() public {
        A.Foo;
    }
}
// ----
// Warning: (69-111): Function state mutability can be restricted to pure
