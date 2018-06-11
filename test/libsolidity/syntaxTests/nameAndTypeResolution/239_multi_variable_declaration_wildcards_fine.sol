contract C {
    function three() public returns (uint, uint, uint);
    function two() public returns (uint, uint);
    function none();
    function f() public {
        (uint a,) = three();
        (uint b, uint c,) = two();
        (,uint d) = three();
        (,uint e, uint g) = two();
        var (,,) = three();
        var () = none();
        a;b;c;d;e;g;
    }
}
// ----
// Warning: (172-191): Different number of components on the left hand side (2) than on the right hand side (3).
// Warning: (201-226): Different number of components on the left hand side (3) than on the right hand side (2).
// Warning: (236-255): Different number of components on the left hand side (2) than on the right hand side (3).
// Warning: (265-290): Different number of components on the left hand side (3) than on the right hand side (2).
// Warning: (121-137): No visibility specified. Defaulting to "public". 
