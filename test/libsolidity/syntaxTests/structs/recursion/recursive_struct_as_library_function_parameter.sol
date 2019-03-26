library Test {
    struct MyStructName {
        address addr;
        MyStructName[] x;
    }

    function f(MyStructName storage s) public {}
}
// ----
