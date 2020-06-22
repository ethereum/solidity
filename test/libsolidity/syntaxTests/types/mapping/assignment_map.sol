contract C {
    mapping (uint => address payable [ ]) public a = a ;
}

contract D {
    mapping (uint => uint) a;
    mapping (uint => uint) b = a;
}

contract F {
    mapping (uint => uint) a;
    mapping (uint => uint) b;

    function foo() public {
        a = b;
    }
}

contract G {
    uint x = 1;
    mapping (uint => uint) b = x;
}
// ----
// TypeError 6280: (17-67): Mappings cannot be assigned to.
// TypeError 6280: (120-148): Mappings cannot be assigned to.
// TypeError 9214: (263-264): Mappings cannot be assigned to.
// TypeError 6280: (312-340): Mappings cannot be assigned to.
