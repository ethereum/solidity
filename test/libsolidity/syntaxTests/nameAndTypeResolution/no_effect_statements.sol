contract test {
 struct s { uint a; uint b;}
    function f() pure public returns (byte) {
		s;
		s(1,2);
		s[7];
		uint;
		uint[7];
    }
}
// ----
// Warning 6133: (93-94): Statement has no effect.
// Warning 6133: (98-104): Statement has no effect.
// Warning 6133: (108-112): Statement has no effect.
// Warning 6133: (116-120): Statement has no effect.
// Warning 6133: (124-131): Statement has no effect.
