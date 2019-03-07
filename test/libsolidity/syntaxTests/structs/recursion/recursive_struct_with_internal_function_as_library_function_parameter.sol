library Test {
    struct MyStructName {
        address addr;
        MyStructName[] x;
        function() internal y;
    }

    function f(MyStructName storage s) public {}
}
// ----
// TypeError: (142-164): Internal type is not allowed for public or external functions.
