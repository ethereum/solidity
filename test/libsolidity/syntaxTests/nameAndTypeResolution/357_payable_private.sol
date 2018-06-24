contract test {
    function f() payable private {}
}
// ----
// TypeError: (20-51): Internal functions cannot be payable.
