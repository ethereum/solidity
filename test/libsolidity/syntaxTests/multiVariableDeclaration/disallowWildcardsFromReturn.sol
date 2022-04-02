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
    function one() public pure returns (uint) {}
    function two() public pure returns (uint, uint) {}
    function three() public pure returns (uint, uint, uint) {}
    function four() public pure returns (uint, uint, uint, uint) {}
    function five() public pure returns (uint, uint, uint, uint, uint) {}
}
// ----
// TypeError 7364: (62-81='(uint a,) = three()'): Different number of components on the left hand side (2) than on the right hand side (3).
// TypeError 7364: (91-110='(,uint b) = three()'): Different number of components on the left hand side (2) than on the right hand side (3).
// TypeError 7364: (120-139='(,uint c,) = five()'): Different number of components on the left hand side (3) than on the right hand side (5).
// TypeError 7364: (149-175='(uint d, uint e,) = four()'): Different number of components on the left hand side (3) than on the right hand side (4).
// TypeError 7364: (185-211='(,uint f, uint g) = four()'): Different number of components on the left hand side (3) than on the right hand side (4).
// TypeError 7364: (221-249='(,uint h, uint i,) = three()'): Different number of components on the left hand side (4) than on the right hand side (3).
// TypeError 7364: (259-276='(uint j,) = one()'): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError 7364: (286-303='(,uint k) = one()'): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError 7364: (313-331='(,uint l,) = one()'): Different number of components on the left hand side (3) than on the right hand side (1).
// TypeError 7364: (341-368='(,uint m, uint n,) = five()'): Different number of components on the left hand side (4) than on the right hand side (5).
