contract test {
 struct s { uint a; uint b;}
    function f() pure public returns (bytes1) {
		s;
		s(1,2);
		s[7];
		uint;
		uint[7];
    }
}
// ----
// Warning 6321: (83-89): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6133: (95-96): Statement has no effect.
// Warning 6133: (100-106): Statement has no effect.
// Warning 6133: (110-114): Statement has no effect.
// Warning 6133: (118-122): Statement has no effect.
// Warning 6133: (126-133): Statement has no effect.
