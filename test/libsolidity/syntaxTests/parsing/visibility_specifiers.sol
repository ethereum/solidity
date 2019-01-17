contract c {
    uint private a;
    uint internal b;
    uint public c;
    uint d;
    function f() public {}
    function f_priv() private {}
    function f_internal() internal {}
}
// ----
// Warning: (58-71): This declaration shadows an existing declaration.
