abstract contract A {
    function f() external virtual;
}
abstract contract B {
    function f() external virtual {}
}
abstract contract C is A, B {
    function f() external virtual override(A, B);
}
abstract contract D is B, A {
    function f() external virtual override(A, B);
}
// ----
// TypeError 4593: (154-199): Overriding an implemented function with an unimplemented function is not allowed.
// TypeError 4593: (236-281): Overriding an implemented function with an unimplemented function is not allowed.
