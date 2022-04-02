library test {
    function f() payable public {}
}
// ----
// TypeError 7708: (19-49='function f() payable public {}'): Library functions cannot be payable.
