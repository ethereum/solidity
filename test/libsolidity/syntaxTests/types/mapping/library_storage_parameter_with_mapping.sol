struct S { mapping(uint => uint)[2] a; }
library L {
    function f(S storage s) public {}
}
// ----
