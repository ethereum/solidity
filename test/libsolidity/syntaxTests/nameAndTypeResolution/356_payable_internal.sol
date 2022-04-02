contract test {
    function f() payable internal {}
}
// ----
// TypeError 5587: (20-52='function f() payable internal {}'): "internal" and "private" functions cannot be payable.
