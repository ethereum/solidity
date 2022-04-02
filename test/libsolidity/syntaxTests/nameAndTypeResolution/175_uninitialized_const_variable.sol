contract Foo {
    uint constant y;
}
// ----
// TypeError 4266: (19-34='uint constant y'): Uninitialized "constant" variable.
