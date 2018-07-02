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
// Warning: (89-111): Function state mutability can be restricted to pure
// Warning: (116-144): Function state mutability can be restricted to pure
// Warning: (149-182): Function state mutability can be restricted to pure
