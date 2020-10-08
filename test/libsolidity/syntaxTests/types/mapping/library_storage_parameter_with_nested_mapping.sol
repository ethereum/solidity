library Test {
    struct Nested { mapping(uint => uint)[2][] a; }
    struct X { Nested n; }
    function f(X storage x) public {}
}
