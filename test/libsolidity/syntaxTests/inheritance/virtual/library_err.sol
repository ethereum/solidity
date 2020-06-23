library L {
    function f() internal pure virtual returns (uint) { return 0; }
}
// ----
// TypeError 7801: (16-79): Library functions cannot be "virtual".
