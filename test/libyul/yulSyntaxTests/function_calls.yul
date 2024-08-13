{
    function f(a) -> b {}
    function g(a, b, c) {}
    function x() {
        g(1, 2, f(3))
        x()
    }
}
// ----
