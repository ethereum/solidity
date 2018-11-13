contract C {
    function f() public {
        uint a = (1,2);
        uint b = (1,2,3);
        uint c = (1,2,3,4);
    }
    function g() public {
        (uint a1, uint b1, uint c1, uint d1) = 1;
        (uint a2, uint b2, uint c2) = 1;
        (uint a3, uint b3) = 1;
    }
    function h() public {
        (uint a1, uint b1, uint c1, uint d1) = (1,2,3);
        (uint a2, uint b2, uint c2) = (1,2,3,4);
    }
}
// ----
// TypeError: (47-61): Different number of components on the left hand side (1) than on the right hand side (2).
// TypeError: (71-87): Different number of components on the left hand side (1) than on the right hand side (3).
// TypeError: (97-115): Different number of components on the left hand side (1) than on the right hand side (4).
// TypeError: (157-197): Different number of components on the left hand side (4) than on the right hand side (1).
// TypeError: (207-238): Different number of components on the left hand side (3) than on the right hand side (1).
// TypeError: (248-270): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError: (312-358): Different number of components on the left hand side (4) than on the right hand side (3).
// TypeError: (368-407): Different number of components on the left hand side (3) than on the right hand side (4).
