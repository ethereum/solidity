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
// TypeError 3656: (47-67): Contract "D" should be marked as abstract.
// TypeError 3656: (68-106): Contract "E" should be marked as abstract.
// DeclarationError 1563: (141-142): Modifier-style base constructor call without arguments.
// TypeError 3656: (107-147): Contract "F" should be marked as abstract.
// TypeError 3656: (192-210): Contract "H" should be marked as abstract.
// TypeError 3656: (211-247): Contract "I" should be marked as abstract.
// DeclarationError 1563: (280-281): Modifier-style base constructor call without arguments.
// TypeError 3656: (248-286): Contract "J" should be marked as abstract.
// TypeError 7927: (61-64): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 7927: (82-85): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 7927: (121-124): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 2973: (141-142): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
// TypeError 7927: (162-165): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 2973: (182-185): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
// TypeError 2973: (280-281): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
// TypeError 2973: (319-322): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
