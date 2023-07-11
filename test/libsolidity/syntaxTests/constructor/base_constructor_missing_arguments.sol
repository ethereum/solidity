contract C {
    constructor(uint, bool) {}
}

contract D is C() {}
contract E is C() { constructor() {} }
contract F is C() { constructor() C {} }
contract G is C() { constructor() C() {} }

contract H is C {}
contract I is C { constructor() {} }
contract J is C { constructor() C {} }
contract K is C { constructor() C() {} }
// ----
// TypeError 3415: (47-67): No arguments passed to the base constructor. Specify the arguments or mark "D" as abstract.
// TypeError 3415: (68-106): No arguments passed to the base constructor. Specify the arguments or mark "E" as abstract.
// DeclarationError 1563: (141-142): Modifier-style base constructor call without arguments.
// TypeError 3415: (107-147): No arguments passed to the base constructor. Specify the arguments or mark "F" as abstract.
// TypeError 3415: (192-210): No arguments passed to the base constructor. Specify the arguments or mark "H" as abstract.
// TypeError 3415: (211-247): No arguments passed to the base constructor. Specify the arguments or mark "I" as abstract.
// DeclarationError 1563: (280-281): Modifier-style base constructor call without arguments.
// TypeError 3415: (248-286): No arguments passed to the base constructor. Specify the arguments or mark "J" as abstract.
// TypeError 7927: (61-64): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 7927: (82-85): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 7927: (121-124): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 2973: (141-142): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
// TypeError 7927: (162-165): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 2973: (182-185): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
// TypeError 2973: (280-281): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
// TypeError 2973: (319-322): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
