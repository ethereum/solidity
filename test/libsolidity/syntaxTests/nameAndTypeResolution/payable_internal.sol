contract test {
    function f() payable internal {}
}
// ----
// TypeError: (20-52): Internal functions cannot be payable.
