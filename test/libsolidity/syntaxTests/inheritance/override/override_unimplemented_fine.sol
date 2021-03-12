interface A {
    function f() external;
}
interface B {
    function f() external;
}
abstract contract C is A, B {
    function f() external virtual override(A, B);
}
// ----
