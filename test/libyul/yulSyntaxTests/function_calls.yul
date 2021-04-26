{
    function f(a:u256) -> b:u256 {}
    function g(a:u256, b:u256, c:u256) {}
    function x() {
        g(1:u256, 2:u256, f(3:u256))
        x()
    }
}
// ----
