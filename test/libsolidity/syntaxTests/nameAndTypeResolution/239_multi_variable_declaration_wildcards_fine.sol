contract C {
    function three() public returns (uint, uint, uint);
    function two() public returns (uint, uint);
    function none();
    function f() public {
        var (a,) = three();
        var (b,c,) = two();
        var (,d) = three();
        var (,e,g) = two();
        var (,,) = three();
        var () = none();
        a;b;c;d;e;g;
    }
}
// ----
// Warning: (177-178): Use of the "var" keyword is deprecated.
// Warning: (205-206): Use of the "var" keyword is deprecated.
// Warning: (207-208): Use of the "var" keyword is deprecated.
// Warning: (234-235): Use of the "var" keyword is deprecated.
// Warning: (262-263): Use of the "var" keyword is deprecated.
// Warning: (264-265): Use of the "var" keyword is deprecated.
// Warning: (172-190): Different number of components on the left hand side (2) than on the right hand side (3).
// Warning: (200-218): Different number of components on the left hand side (3) than on the right hand side (2).
// Warning: (228-246): Different number of components on the left hand side (2) than on the right hand side (3).
// Warning: (256-274): Different number of components on the left hand side (3) than on the right hand side (2).
// Warning: (121-137): No visibility specified. Defaulting to "public". 
