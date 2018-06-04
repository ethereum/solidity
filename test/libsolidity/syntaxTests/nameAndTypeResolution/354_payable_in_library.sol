library test {
    function f() payable public {}
}
// ----
// TypeError: (19-49): Library functions cannot be payable.
