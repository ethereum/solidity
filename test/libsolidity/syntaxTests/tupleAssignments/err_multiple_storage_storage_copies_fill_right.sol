contract C {
	struct S { uint a; uint b; }
	S x; S y;
	function f() public {
		(x, y, ) = (y, x, 1, 2);
	}
}
// ----
// TypeError: (90-102): Type tuple(struct C.S storage ref,struct C.S storage ref,int_const 1,int_const 2) is not implicitly convertible to expected type tuple(struct C.S storage ref,struct C.S storage ref,).
// Warning: (79-102): This assignment performs two copies to storage. Since storage copies do not first copy to a temporary location, one of them might be overwritten before the second is executed and thus may have unexpected effects. It is safer to perform the copies separately or assign to storage pointers first.
