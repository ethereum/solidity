contract c {
    uint private a;
    uint internal b;
    uint public c;
    uint d;
    function f() {}
    function f_priv() private {}
    function f_public() public {}
    function f_internal() internal {}
}
// ----
// Warning: (58-71): This declaration shadows an existing declaration.
// Warning: (89-104): No visibility specified. Defaulting to "public". 
// Warning: (89-104): Function state mutability can be restricted to pure
// Warning: (109-137): Function state mutability can be restricted to pure
// Warning: (142-171): Function state mutability can be restricted to pure
// Warning: (176-209): Function state mutability can be restricted to pure
