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

contract H {
    struct S {mapping (uint => uint) a;}

	S x;
	S y = x;
	S z = z;
}
// ----
// TypeError 6280: (17-67): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 6280: (120-148): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 9214: (263-264): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 6280: (312-340): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 6280: (407-414): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 6280: (417-424): Types in storage containing (nested) mappings cannot be assigned to.
