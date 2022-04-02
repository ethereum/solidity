contract C {
    function fn() public pure {
        (uint a,) = (1,2,3);
        (,uint b) = (1,2,3);
        (,uint c,) = (1,2,3,4,5);
        (uint d, uint e,) = (1,2,3,4);
        (,uint f, uint g) = (1,2,3,4);
        (,uint h, uint i,) = (1,2,3);
        (uint j,) = 1;
        (,uint k) = 1;
        (,uint l,) = 1;
        a;b;c;d;e;f;g;h;i;j;k;l;
    }
}
// ----
// TypeError 7364: (53-72='(uint a,) = (1,2,3)'): Different number of components on the left hand side (2) than on the right hand side (3).
// TypeError 7364: (82-101='(,uint b) = (1,2,3)'): Different number of components on the left hand side (2) than on the right hand side (3).
// TypeError 7364: (111-135='(,uint c,) = (1,2,3,4,5)'): Different number of components on the left hand side (3) than on the right hand side (5).
// TypeError 7364: (145-174='(uint d, uint e,) = (1,2,3,4)'): Different number of components on the left hand side (3) than on the right hand side (4).
// TypeError 7364: (184-213='(,uint f, uint g) = (1,2,3,4)'): Different number of components on the left hand side (3) than on the right hand side (4).
// TypeError 7364: (223-251='(,uint h, uint i,) = (1,2,3)'): Different number of components on the left hand side (4) than on the right hand side (3).
// TypeError 7364: (261-274='(uint j,) = 1'): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError 7364: (284-297='(,uint k) = 1'): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError 7364: (307-321='(,uint l,) = 1'): Different number of components on the left hand side (3) than on the right hand side (1).
