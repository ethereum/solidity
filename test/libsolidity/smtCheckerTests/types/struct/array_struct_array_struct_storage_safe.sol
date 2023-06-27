pragma abicoder               v2;

contract C {
	struct T {
		uint y;
		uint[] a;
	}
	struct S {
		uint x;
		T t;
		uint[] a;
		T[] ts;
	}
	S[] s1;
	function f() public {
		s1.push();
		s1.push();
		s1.push();
		s1[0].x = 2;
		assert(s1[0].x == 2);
		s1[1].t.y = 3;
		assert(s1[1].t.y == 3);
		s1[2].a.push();
		s1[2].a.push();
		s1[2].a.push();
		s1[2].a[2] = 4;
		assert(s1[2].a[2] == 4);
		s1[0].ts.push();
		s1[0].ts.push();
		s1[0].ts.push();
		s1[0].ts.push();
		s1[0].ts.push();
		s1[0].ts[3].y = 5;
		assert(s1[0].ts[3].y == 5);
		s1[1].ts.push();
		s1[1].ts.push();
		s1[1].ts.push();
		s1[1].ts.push();
		s1[1].ts.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a.push();
		s1[1].ts[4].a[5] = 6;
		assert(s1[1].ts[4].a[5] == 6);
		s1.pop();
		s1.pop();
		s1.pop();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (212-217): CHC: Out of bounds access might happen here.
// Warning 6368: (234-239): CHC: Out of bounds access might happen here.
// Warning 6328: (227-247): CHC: Assertion violation might happen here.
// Warning 6368: (251-256): CHC: Out of bounds access might happen here.
// Warning 6368: (275-280): CHC: Out of bounds access might happen here.
// Warning 6328: (268-290): CHC: Assertion violation might happen here.
// Warning 6368: (294-299): CHC: Out of bounds access might happen here.
// Warning 6368: (312-317): CHC: Out of bounds access might happen here.
// Warning 6368: (330-335): CHC: Out of bounds access might happen here.
// Warning 6368: (348-353): CHC: Out of bounds access might happen here.
// Warning 6368: (348-358): CHC: Out of bounds access might happen here.
// Warning 6368: (373-378): CHC: Out of bounds access might happen here.
// Warning 6368: (373-383): CHC: Out of bounds access might happen here.
// Warning 6328: (366-389): CHC: Assertion violation might happen here.
// Warning 6368: (393-398): CHC: Out of bounds access might happen here.
// Warning 6368: (412-417): CHC: Out of bounds access might happen here.
// Warning 6368: (431-436): CHC: Out of bounds access might happen here.
// Warning 6368: (450-455): CHC: Out of bounds access might happen here.
// Warning 6368: (469-474): CHC: Out of bounds access might happen here.
// Warning 6368: (488-493): CHC: Out of bounds access might happen here.
// Warning 6368: (488-499): CHC: Out of bounds access might happen here.
// Warning 6368: (516-521): CHC: Out of bounds access might happen here.
// Warning 6368: (516-527): CHC: Out of bounds access might happen here.
// Warning 6328: (509-535): CHC: Assertion violation might happen here.
// Warning 6368: (539-544): CHC: Out of bounds access might happen here.
// Warning 6368: (558-563): CHC: Out of bounds access might happen here.
// Warning 6368: (577-582): CHC: Out of bounds access might happen here.
// Warning 6368: (596-601): CHC: Out of bounds access might happen here.
// Warning 6368: (615-620): CHC: Out of bounds access might happen here.
// Warning 6368: (634-639): CHC: Out of bounds access might happen here.
// Warning 6368: (634-645): CHC: Out of bounds access might happen here.
// Warning 6368: (658-663): CHC: Out of bounds access might happen here.
// Warning 6368: (658-669): CHC: Out of bounds access might happen here.
// Warning 6368: (682-687): CHC: Out of bounds access might happen here.
// Warning 6368: (682-693): CHC: Out of bounds access might happen here.
// Warning 6368: (706-711): CHC: Out of bounds access might happen here.
// Warning 6368: (706-717): CHC: Out of bounds access might happen here.
// Warning 6368: (730-735): CHC: Out of bounds access might happen here.
// Warning 6368: (730-741): CHC: Out of bounds access might happen here.
// Warning 6368: (754-759): CHC: Out of bounds access might happen here.
// Warning 6368: (754-765): CHC: Out of bounds access might happen here.
// Warning 6368: (778-783): CHC: Out of bounds access might happen here.
// Warning 6368: (778-789): CHC: Out of bounds access might happen here.
// Warning 6368: (778-794): CHC: Out of bounds access might happen here.
// Warning 6368: (809-814): CHC: Out of bounds access might happen here.
// Warning 6368: (809-820): CHC: Out of bounds access might happen here.
// Warning 6368: (809-825): CHC: Out of bounds access might happen here.
// Warning 6328: (802-831): CHC: Assertion violation might happen here.
// Warning 2529: (835-843): CHC: Empty array "pop" might happen here.
// Warning 2529: (847-855): CHC: Empty array "pop" might happen here.
// Warning 2529: (859-867): CHC: Empty array "pop" might happen here.
// Info 6002: BMC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
