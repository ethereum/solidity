pragma experimental SMTChecker;

contract C {
	struct S {
		uint x;
		S[] a;
	}
	S s1;
	S s2;
	function f() public view {
		assert(s1.x == s2.x);
		assert(s1.a.length == s2.a.length);
		assert(s1.a[0].x == s2.a[0].x);
		assert(s1.a[0].a.length == s2.a[0].a.length);
		assert(s1.a[0].a[0].x == s2.a[0].a[0].x);
	}
	function g() public {
		s1.x = 42;
		s2.x = 42;
		s1.a.push();
		s2.a.push();
		s1.a[0].x = 43;
		s2.a[0].x = 43;
		s1.a[0].a.push();
		s2.a[0].a.push();
		s1.a[0].a[0].x = 44;
		s2.a[0].a[0].x = 44;
	}
}
// ----
// Warning 8115: (81-85): Assertion checker does not yet support the type of this variable.
// Warning 8115: (88-92): Assertion checker does not yet support the type of this variable.
// Warning 7650: (131-135): Assertion checker does not yet support this expression.
// Warning 8364: (131-133): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (139-143): Assertion checker does not yet support this expression.
// Warning 8364: (139-141): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (155-159): Assertion checker does not yet support this expression.
// Warning 8364: (155-157): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (170-174): Assertion checker does not yet support this expression.
// Warning 8364: (170-172): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (193-202): Assertion checker does not yet support this expression.
// Warning 7650: (193-197): Assertion checker does not yet support this expression.
// Warning 8364: (193-195): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (193-200): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (206-215): Assertion checker does not yet support this expression.
// Warning 7650: (206-210): Assertion checker does not yet support this expression.
// Warning 8364: (206-208): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (206-213): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (227-236): Assertion checker does not yet support this expression.
// Warning 7650: (227-231): Assertion checker does not yet support this expression.
// Warning 8364: (227-229): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (227-234): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (247-256): Assertion checker does not yet support this expression.
// Warning 7650: (247-251): Assertion checker does not yet support this expression.
// Warning 8364: (247-249): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (247-254): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (275-289): Assertion checker does not yet support this expression.
// Warning 7650: (275-284): Assertion checker does not yet support this expression.
// Warning 7650: (275-279): Assertion checker does not yet support this expression.
// Warning 8364: (275-277): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (275-282): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (275-287): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (293-307): Assertion checker does not yet support this expression.
// Warning 7650: (293-302): Assertion checker does not yet support this expression.
// Warning 7650: (293-297): Assertion checker does not yet support this expression.
// Warning 8364: (293-295): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (293-300): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (293-305): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (338-342): Assertion checker does not yet support this expression.
// Warning 8364: (338-340): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (338-342): Assertion checker does not support recursive structs.
// Warning 7650: (351-355): Assertion checker does not yet support this expression.
// Warning 8364: (351-353): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (351-355): Assertion checker does not support recursive structs.
// Warning 7650: (364-368): Assertion checker does not yet support this expression.
// Warning 8364: (364-366): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (364-375): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (364-368): Assertion checker does not support recursive structs.
// Warning 7650: (379-383): Assertion checker does not yet support this expression.
// Warning 8364: (379-381): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (379-390): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (379-383): Assertion checker does not support recursive structs.
// Warning 7650: (394-403): Assertion checker does not yet support this expression.
// Warning 7650: (394-398): Assertion checker does not yet support this expression.
// Warning 8364: (394-396): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (394-401): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (394-403): Assertion checker does not support recursive structs.
// Warning 7650: (412-421): Assertion checker does not yet support this expression.
// Warning 7650: (412-416): Assertion checker does not yet support this expression.
// Warning 8364: (412-414): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (412-419): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (412-421): Assertion checker does not support recursive structs.
// Warning 7650: (430-439): Assertion checker does not yet support this expression.
// Warning 7650: (430-434): Assertion checker does not yet support this expression.
// Warning 8364: (430-432): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (430-437): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (430-446): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (430-439): Assertion checker does not support recursive structs.
// Warning 7650: (450-459): Assertion checker does not yet support this expression.
// Warning 7650: (450-454): Assertion checker does not yet support this expression.
// Warning 8364: (450-452): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (450-457): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (450-466): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (450-459): Assertion checker does not support recursive structs.
// Warning 7650: (470-484): Assertion checker does not yet support this expression.
// Warning 7650: (470-479): Assertion checker does not yet support this expression.
// Warning 7650: (470-474): Assertion checker does not yet support this expression.
// Warning 8364: (470-472): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (470-477): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (470-482): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (470-484): Assertion checker does not support recursive structs.
// Warning 7650: (493-507): Assertion checker does not yet support this expression.
// Warning 7650: (493-502): Assertion checker does not yet support this expression.
// Warning 7650: (493-497): Assertion checker does not yet support this expression.
// Warning 8364: (493-495): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (493-500): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (493-505): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (493-507): Assertion checker does not support recursive structs.
// Warning 6328: (124-144): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (148-182): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (186-216): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (220-264): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (268-308): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 8115: (81-85): Assertion checker does not yet support the type of this variable.
// Warning 8115: (88-92): Assertion checker does not yet support the type of this variable.
// Warning 7650: (131-135): Assertion checker does not yet support this expression.
// Warning 8364: (131-133): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (139-143): Assertion checker does not yet support this expression.
// Warning 8364: (139-141): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (155-159): Assertion checker does not yet support this expression.
// Warning 8364: (155-157): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (170-174): Assertion checker does not yet support this expression.
// Warning 8364: (170-172): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (193-202): Assertion checker does not yet support this expression.
// Warning 7650: (193-197): Assertion checker does not yet support this expression.
// Warning 8364: (193-195): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (193-200): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (206-215): Assertion checker does not yet support this expression.
// Warning 7650: (206-210): Assertion checker does not yet support this expression.
// Warning 8364: (206-208): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (206-213): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (227-236): Assertion checker does not yet support this expression.
// Warning 7650: (227-231): Assertion checker does not yet support this expression.
// Warning 8364: (227-229): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (227-234): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (247-256): Assertion checker does not yet support this expression.
// Warning 7650: (247-251): Assertion checker does not yet support this expression.
// Warning 8364: (247-249): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (247-254): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (275-289): Assertion checker does not yet support this expression.
// Warning 7650: (275-284): Assertion checker does not yet support this expression.
// Warning 7650: (275-279): Assertion checker does not yet support this expression.
// Warning 8364: (275-277): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (275-282): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (275-287): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (293-307): Assertion checker does not yet support this expression.
// Warning 7650: (293-302): Assertion checker does not yet support this expression.
// Warning 7650: (293-297): Assertion checker does not yet support this expression.
// Warning 8364: (293-295): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (293-300): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (293-305): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (338-342): Assertion checker does not yet support this expression.
// Warning 8364: (338-340): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (338-342): Assertion checker does not support recursive structs.
// Warning 7650: (351-355): Assertion checker does not yet support this expression.
// Warning 8364: (351-353): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (351-355): Assertion checker does not support recursive structs.
// Warning 7650: (364-368): Assertion checker does not yet support this expression.
// Warning 8364: (364-366): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (364-375): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (364-368): Assertion checker does not support recursive structs.
// Warning 7650: (379-383): Assertion checker does not yet support this expression.
// Warning 8364: (379-381): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (379-390): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (379-383): Assertion checker does not support recursive structs.
// Warning 7650: (394-403): Assertion checker does not yet support this expression.
// Warning 7650: (394-398): Assertion checker does not yet support this expression.
// Warning 8364: (394-396): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (394-401): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (394-403): Assertion checker does not support recursive structs.
// Warning 7650: (412-421): Assertion checker does not yet support this expression.
// Warning 7650: (412-416): Assertion checker does not yet support this expression.
// Warning 8364: (412-414): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (412-419): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (412-421): Assertion checker does not support recursive structs.
// Warning 7650: (430-439): Assertion checker does not yet support this expression.
// Warning 7650: (430-434): Assertion checker does not yet support this expression.
// Warning 8364: (430-432): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (430-437): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (430-446): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (430-439): Assertion checker does not support recursive structs.
// Warning 7650: (450-459): Assertion checker does not yet support this expression.
// Warning 7650: (450-454): Assertion checker does not yet support this expression.
// Warning 8364: (450-452): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (450-457): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (450-466): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (450-459): Assertion checker does not support recursive structs.
// Warning 7650: (470-484): Assertion checker does not yet support this expression.
// Warning 7650: (470-479): Assertion checker does not yet support this expression.
// Warning 7650: (470-474): Assertion checker does not yet support this expression.
// Warning 8364: (470-472): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (470-477): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (470-482): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (470-484): Assertion checker does not support recursive structs.
// Warning 7650: (493-507): Assertion checker does not yet support this expression.
// Warning 7650: (493-502): Assertion checker does not yet support this expression.
// Warning 7650: (493-497): Assertion checker does not yet support this expression.
// Warning 8364: (493-495): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (493-500): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (493-505): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (493-507): Assertion checker does not support recursive structs.
