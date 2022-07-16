contract A {
    function f() pure public {
        bytes1 b;
        bytes1 a;
        (((, , b))) = (1, 2, a);
    }
}
