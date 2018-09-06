contract C {
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
// TypeError: (53-72): Different number of components on the left hand side (2) than on the right hand side (3).
// TypeError: (82-101): Different number of components on the left hand side (2) than on the right hand side (3).
// TypeError: (111-130): Different number of components on the left hand side (3) than on the right hand side (5).
// TypeError: (140-166): Different number of components on the left hand side (3) than on the right hand side (4).
// TypeError: (176-202): Different number of components on the left hand side (3) than on the right hand side (4).
// TypeError: (212-240): Different number of components on the left hand side (4) than on the right hand side (3).
// TypeError: (250-267): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError: (277-294): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError: (304-322): Different number of components on the left hand side (3) than on the right hand side (1).
// TypeError: (332-359): Different number of components on the left hand side (4) than on the right hand side (5).
