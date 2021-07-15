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
		require(s1.a.length > 0);
		require(s2.a.length > 0);
		assert(s1.a[0].x == s2.a[0].x);
		require(s1.a[0].a.length > 0);
		require(s2.a[0].a.length > 0);
		assert(s1.a[0].a.length == s2.a[0].a.length);
		require(s1.a[0].a[0].a.length > 0);
		require(s2.a[0].a[0].a.length > 0);
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
// ====
// SMTEngine: all
// ----
// Warning 8115: (48-52): Assertion checker does not yet support the type of this variable.
// Warning 8115: (55-59): Assertion checker does not yet support the type of this variable.
// Warning 7650: (98-102): Assertion checker does not yet support this expression.
// Warning 8364: (98-100): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (106-110): Assertion checker does not yet support this expression.
// Warning 8364: (106-108): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (122-126): Assertion checker does not yet support this expression.
// Warning 8364: (122-124): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (137-141): Assertion checker does not yet support this expression.
// Warning 8364: (137-139): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (161-165): Assertion checker does not yet support this expression.
// Warning 8364: (161-163): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (189-193): Assertion checker does not yet support this expression.
// Warning 8364: (189-191): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (216-225): Assertion checker does not yet support this expression.
// Warning 7650: (216-220): Assertion checker does not yet support this expression.
// Warning 8364: (216-218): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (216-223): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (229-238): Assertion checker does not yet support this expression.
// Warning 7650: (229-233): Assertion checker does not yet support this expression.
// Warning 8364: (229-231): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (229-236): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (251-260): Assertion checker does not yet support this expression.
// Warning 7650: (251-255): Assertion checker does not yet support this expression.
// Warning 8364: (251-253): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (251-258): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (284-293): Assertion checker does not yet support this expression.
// Warning 7650: (284-288): Assertion checker does not yet support this expression.
// Warning 8364: (284-286): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (284-291): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (316-325): Assertion checker does not yet support this expression.
// Warning 7650: (316-320): Assertion checker does not yet support this expression.
// Warning 8364: (316-318): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (316-323): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (336-345): Assertion checker does not yet support this expression.
// Warning 7650: (336-340): Assertion checker does not yet support this expression.
// Warning 8364: (336-338): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (336-343): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (365-379): Assertion checker does not yet support this expression.
// Warning 7650: (365-374): Assertion checker does not yet support this expression.
// Warning 7650: (365-369): Assertion checker does not yet support this expression.
// Warning 8364: (365-367): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (365-372): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (365-377): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (403-417): Assertion checker does not yet support this expression.
// Warning 7650: (403-412): Assertion checker does not yet support this expression.
// Warning 7650: (403-407): Assertion checker does not yet support this expression.
// Warning 8364: (403-405): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (403-410): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (403-415): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (440-454): Assertion checker does not yet support this expression.
// Warning 7650: (440-449): Assertion checker does not yet support this expression.
// Warning 7650: (440-444): Assertion checker does not yet support this expression.
// Warning 8364: (440-442): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (440-447): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (440-452): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (458-472): Assertion checker does not yet support this expression.
// Warning 7650: (458-467): Assertion checker does not yet support this expression.
// Warning 7650: (458-462): Assertion checker does not yet support this expression.
// Warning 8364: (458-460): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (458-465): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (458-470): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (503-507): Assertion checker does not yet support this expression.
// Warning 8364: (503-505): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (503-507): Assertion checker does not support recursive structs.
// Warning 7650: (516-520): Assertion checker does not yet support this expression.
// Warning 8364: (516-518): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (516-520): Assertion checker does not support recursive structs.
// Warning 7650: (529-533): Assertion checker does not yet support this expression.
// Warning 8364: (529-531): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (529-540): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (529-533): Assertion checker does not support recursive structs.
// Warning 7650: (544-548): Assertion checker does not yet support this expression.
// Warning 8364: (544-546): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (544-555): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (544-548): Assertion checker does not support recursive structs.
// Warning 7650: (559-568): Assertion checker does not yet support this expression.
// Warning 7650: (559-563): Assertion checker does not yet support this expression.
// Warning 8364: (559-561): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (559-566): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (559-568): Assertion checker does not support recursive structs.
// Warning 7650: (577-586): Assertion checker does not yet support this expression.
// Warning 7650: (577-581): Assertion checker does not yet support this expression.
// Warning 8364: (577-579): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (577-584): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (577-586): Assertion checker does not support recursive structs.
// Warning 7650: (595-604): Assertion checker does not yet support this expression.
// Warning 7650: (595-599): Assertion checker does not yet support this expression.
// Warning 8364: (595-597): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (595-602): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (595-611): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (595-604): Assertion checker does not support recursive structs.
// Warning 7650: (615-624): Assertion checker does not yet support this expression.
// Warning 7650: (615-619): Assertion checker does not yet support this expression.
// Warning 8364: (615-617): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (615-622): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (615-631): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (615-624): Assertion checker does not support recursive structs.
// Warning 7650: (635-649): Assertion checker does not yet support this expression.
// Warning 7650: (635-644): Assertion checker does not yet support this expression.
// Warning 7650: (635-639): Assertion checker does not yet support this expression.
// Warning 8364: (635-637): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (635-642): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (635-647): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (635-649): Assertion checker does not support recursive structs.
// Warning 7650: (658-672): Assertion checker does not yet support this expression.
// Warning 7650: (658-667): Assertion checker does not yet support this expression.
// Warning 7650: (658-662): Assertion checker does not yet support this expression.
// Warning 8364: (658-660): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (658-665): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (658-670): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (658-672): Assertion checker does not support recursive structs.
// Warning 6328: (91-111): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (115-149): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (216-223): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (229-236): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (209-239): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (251-258): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (284-291): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (316-323): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (336-343): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (309-353): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (365-372): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (365-377): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (403-410): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (403-415): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (440-447): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (440-452): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (458-465): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (458-470): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (433-473): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6368: (559-566): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (577-584): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (595-602): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (615-622): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (635-642): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (635-647): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (658-665): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 6368: (658-670): CHC: Out of bounds access happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g()
// Warning 8115: (48-52): Assertion checker does not yet support the type of this variable.
// Warning 8115: (55-59): Assertion checker does not yet support the type of this variable.
// Warning 7650: (98-102): Assertion checker does not yet support this expression.
// Warning 8364: (98-100): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (106-110): Assertion checker does not yet support this expression.
// Warning 8364: (106-108): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (122-126): Assertion checker does not yet support this expression.
// Warning 8364: (122-124): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (137-141): Assertion checker does not yet support this expression.
// Warning 8364: (137-139): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (161-165): Assertion checker does not yet support this expression.
// Warning 8364: (161-163): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (189-193): Assertion checker does not yet support this expression.
// Warning 8364: (189-191): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (216-225): Assertion checker does not yet support this expression.
// Warning 7650: (216-220): Assertion checker does not yet support this expression.
// Warning 8364: (216-218): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (216-223): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (229-238): Assertion checker does not yet support this expression.
// Warning 7650: (229-233): Assertion checker does not yet support this expression.
// Warning 8364: (229-231): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (229-236): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (251-260): Assertion checker does not yet support this expression.
// Warning 7650: (251-255): Assertion checker does not yet support this expression.
// Warning 8364: (251-253): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (251-258): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (284-293): Assertion checker does not yet support this expression.
// Warning 7650: (284-288): Assertion checker does not yet support this expression.
// Warning 8364: (284-286): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (284-291): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (316-325): Assertion checker does not yet support this expression.
// Warning 7650: (316-320): Assertion checker does not yet support this expression.
// Warning 8364: (316-318): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (316-323): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (336-345): Assertion checker does not yet support this expression.
// Warning 7650: (336-340): Assertion checker does not yet support this expression.
// Warning 8364: (336-338): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (336-343): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (365-379): Assertion checker does not yet support this expression.
// Warning 7650: (365-374): Assertion checker does not yet support this expression.
// Warning 7650: (365-369): Assertion checker does not yet support this expression.
// Warning 8364: (365-367): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (365-372): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (365-377): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (403-417): Assertion checker does not yet support this expression.
// Warning 7650: (403-412): Assertion checker does not yet support this expression.
// Warning 7650: (403-407): Assertion checker does not yet support this expression.
// Warning 8364: (403-405): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (403-410): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (403-415): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (440-454): Assertion checker does not yet support this expression.
// Warning 7650: (440-449): Assertion checker does not yet support this expression.
// Warning 7650: (440-444): Assertion checker does not yet support this expression.
// Warning 8364: (440-442): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (440-447): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (440-452): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (458-472): Assertion checker does not yet support this expression.
// Warning 7650: (458-467): Assertion checker does not yet support this expression.
// Warning 7650: (458-462): Assertion checker does not yet support this expression.
// Warning 8364: (458-460): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (458-465): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (458-470): Assertion checker does not yet implement type struct C.S storage ref
// Warning 7650: (503-507): Assertion checker does not yet support this expression.
// Warning 8364: (503-505): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (503-507): Assertion checker does not support recursive structs.
// Warning 7650: (516-520): Assertion checker does not yet support this expression.
// Warning 8364: (516-518): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (516-520): Assertion checker does not support recursive structs.
// Warning 7650: (529-533): Assertion checker does not yet support this expression.
// Warning 8364: (529-531): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (529-540): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (529-533): Assertion checker does not support recursive structs.
// Warning 7650: (544-548): Assertion checker does not yet support this expression.
// Warning 8364: (544-546): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (544-555): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (544-548): Assertion checker does not support recursive structs.
// Warning 7650: (559-568): Assertion checker does not yet support this expression.
// Warning 7650: (559-563): Assertion checker does not yet support this expression.
// Warning 8364: (559-561): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (559-566): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (559-568): Assertion checker does not support recursive structs.
// Warning 7650: (577-586): Assertion checker does not yet support this expression.
// Warning 7650: (577-581): Assertion checker does not yet support this expression.
// Warning 8364: (577-579): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (577-584): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (577-586): Assertion checker does not support recursive structs.
// Warning 7650: (595-604): Assertion checker does not yet support this expression.
// Warning 7650: (595-599): Assertion checker does not yet support this expression.
// Warning 8364: (595-597): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (595-602): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (595-611): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (595-604): Assertion checker does not support recursive structs.
// Warning 7650: (615-624): Assertion checker does not yet support this expression.
// Warning 7650: (615-619): Assertion checker does not yet support this expression.
// Warning 8364: (615-617): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (615-622): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (615-631): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (615-624): Assertion checker does not support recursive structs.
// Warning 7650: (635-649): Assertion checker does not yet support this expression.
// Warning 7650: (635-644): Assertion checker does not yet support this expression.
// Warning 7650: (635-639): Assertion checker does not yet support this expression.
// Warning 8364: (635-637): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (635-642): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (635-647): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (635-649): Assertion checker does not support recursive structs.
// Warning 7650: (658-672): Assertion checker does not yet support this expression.
// Warning 7650: (658-667): Assertion checker does not yet support this expression.
// Warning 7650: (658-662): Assertion checker does not yet support this expression.
// Warning 8364: (658-660): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (658-665): Assertion checker does not yet implement type struct C.S storage ref
// Warning 8364: (658-670): Assertion checker does not yet implement type struct C.S storage ref
// Warning 4375: (658-672): Assertion checker does not support recursive structs.
