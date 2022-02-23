library Test {
    struct Nested { mapping(uint => uint)[2][] a; uint y; }
    struct X { Nested n; }
    function f(X storage x) external {}
}
// ----
