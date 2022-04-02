contract test {
    function f() payable private {}
}
// ----
// TypeError 5587: (20-51='function f() payable private {}'): "internal" and "private" functions cannot be payable.
