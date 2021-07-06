contract A {
    function f() public virtual {}
}
abstract contract B {
    function f() public virtual;
}
abstract contract C is A, B {
    function g() public {
        f(); // Would call B.f() if we did not require an override in C.
    }
}
// ----
// TypeError 6480: (107-243): Derived contract must override function "f". Two or more base classes define function with same name and parameter types.
