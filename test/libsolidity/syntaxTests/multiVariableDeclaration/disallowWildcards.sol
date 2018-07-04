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
// TypeError: (53-72): Different number of components on the left hand side (2) than on the right hand side (3).
// TypeError: (82-101): Different number of components on the left hand side (2) than on the right hand side (3).
// TypeError: (111-135): Different number of components on the left hand side (3) than on the right hand side (5).
// TypeError: (145-174): Different number of components on the left hand side (3) than on the right hand side (4).
// TypeError: (184-213): Different number of components on the left hand side (3) than on the right hand side (4).
// TypeError: (223-251): Different number of components on the left hand side (4) than on the right hand side (3).
// TypeError: (261-274): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError: (284-297): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError: (307-321): Different number of components on the left hand side (3) than on the right hand side (1).
