abstract contract C {
    function f() public {
        uint a = two();
        uint b = three();
        uint c = four();
    }
    function g() public {
        (uint a1, uint b1, uint c1, uint d1) = one();
        (uint a2, uint b2, uint c2) = one();
        (uint a3, uint b3) = one();
    }
    function h() public {
        (uint a1, uint b1, uint c1, uint d1) = three();
        (uint a2, uint b2, uint c2) = four();
    }
    function one() public pure returns (uint);
    function two() public pure returns (uint, uint);
    function three() public pure returns (uint, uint, uint);
    function four() public pure returns (uint, uint, uint, uint);
}
// ----
// TypeError: (56-70): Different number of components on the left hand side (1) than on the right hand side (2).
// TypeError: (80-96): Different number of components on the left hand side (1) than on the right hand side (3).
// TypeError: (106-121): Different number of components on the left hand side (1) than on the right hand side (4).
// TypeError: (163-207): Different number of components on the left hand side (4) than on the right hand side (1).
// TypeError: (217-252): Different number of components on the left hand side (3) than on the right hand side (1).
// TypeError: (262-288): Different number of components on the left hand side (2) than on the right hand side (1).
// TypeError: (330-376): Different number of components on the left hand side (4) than on the right hand side (3).
// TypeError: (386-422): Different number of components on the left hand side (3) than on the right hand side (4).
