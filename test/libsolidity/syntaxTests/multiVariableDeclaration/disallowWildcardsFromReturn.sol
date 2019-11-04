abstract contract C {
    function fn() public pure {
        (uint a,) = three();
        (,uint b) = three();
        (,uint c,) = five();
        (uint d, uint e,) = four();
        (,uint f, uint g) = four();
        (,uint h, uint i,) = three();
        (uint j,) = one();
        (,uint k) = one();
        (,uint l,) = one();
        (,uint m, uint n,) = five();
        a;b;c;d;e;f;g;h;i;j;k;l;m;n;
    }
    function one() public pure returns (uint);
    function two() public pure returns (uint, uint);
    function three() public pure returns (uint, uint, uint);
    function four() public pure returns (uint, uint, uint, uint);
    function five() public pure returns (uint, uint, uint, uint, uint);
}
// ----
// TypeError: (62-81): Different number of components on the left hand side (2) than on the right hand side (3).
// TypeError: (91-110): Different number of components on the left hand side (2) than on the right hand side (3).
// TypeError: (120-139): Different number of components on the left hand side (3) than on the right hand side (5).
// TypeError: (149-175): Different number of components on the left hand side (3) than on the right hand side (4).
// TypeError: (185-211): Different number of components on the left hand side (3) than on the right hand side (4).
// TypeError: (221-249): Different number of components on the left hand side (4) than on the right hand side (3).
// TypeError: (259-276): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError: (286-303): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError: (313-331): Different number of components on the left hand side (3) than on the right hand side (1).
// TypeError: (341-368): Different number of components on the left hand side (4) than on the right hand side (5).
