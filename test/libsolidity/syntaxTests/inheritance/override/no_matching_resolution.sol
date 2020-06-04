contract A {
    function f() virtual internal {}
}
contract B is A {
    function f() virtual override internal {}
    function h() pure internal { f; }
}
contract C is B {
    function f() override internal {}
    function i() pure internal { f; }
}
