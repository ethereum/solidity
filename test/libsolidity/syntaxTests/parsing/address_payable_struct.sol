contract C {
    struct S {
        address payable a;
        address payable[] b;
        mapping(uint => address payable) c;
        mapping(uint => address payable[]) d;
    }
}