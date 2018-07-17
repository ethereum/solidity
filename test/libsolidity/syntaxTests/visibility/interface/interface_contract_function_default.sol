// State of the syntax checker has to be reset after the interface
// was visited. The suggested visibility for g() should not be external.
interface I {
    function f();
}
contract C {
    function g();
}
// ----
// SyntaxError: (158-171): No visibility specified. Did you intend to add "external"?
// SyntaxError: (191-204): No visibility specified. Did you intend to add "public"?
// TypeError: (158-171): Functions in interfaces must be declared external.
