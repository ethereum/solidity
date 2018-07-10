contract C {
	struct S { uint a; uint b; }
	S x; S y;
	function f() public {
		(,x, y) = (1, 2, y, x);
	}
}
// ----
// TypeError: (89-101): Type tuple(int_const 1,int_const 2,struct C.S storage ref,struct C.S storage ref) is not implicitly convertible to expected type tuple(,struct C.S storage ref,struct C.S storage ref).
// Warning: (79-101): This assignment performs two copies to storage. Since storage copies do not first copy to a temporary location, one of them might be overwritten before the second is executed and thus may have unexpected effects. It is safer to perform the copies separately or assign to storage pointers first.
