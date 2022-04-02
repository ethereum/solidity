// State of the syntax checker has to be reset after the interface
// was visited. The suggested visibility for g() should not be external.
interface I {
    function f();
}
abstract contract C {
    function g() {}
}
// ----
// SyntaxError 4937: (158-171='function f();'): No visibility specified. Did you intend to add "external"?
// SyntaxError 4937: (200-215='function g() {}'): No visibility specified. Did you intend to add "public"?
// TypeError 1560: (158-171='function f();'): Functions in interfaces must be declared external.
