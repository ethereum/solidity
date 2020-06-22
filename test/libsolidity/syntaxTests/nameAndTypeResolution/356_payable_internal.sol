contract test {
    function f() payable internal {}
}
// ----
// TypeError 5587: (20-52): Internal functions cannot be payable.
