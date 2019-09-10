contract Test {
    struct MyStructName {
        address addr;
        MyStructName[] x;
    }

    function f(MyStructName memory s) public {}
}
// ----
// TypeError: (112-133): Recursive type not allowed for public or external contract functions.
