contract C {
	function f(string calldata data) external pure {
		C c;
		assert(this == this);

		assert(this == c);
		assert(this != c);
		assert(this >= c);
		assert(this <= c);
		assert(this < c);
		assert(this > c);
	}
}
// ----
// TypeError 2271: (79-91): Operator == not compatible with types contract C and contract C. No arithmetic or comparison operations are allowed on contract types. Consider converting to "address".
// TypeError 2271: (104-113): Operator == not compatible with types contract C and contract C. No arithmetic or comparison operations are allowed on contract types. Consider converting to "address".
// TypeError 2271: (125-134): Operator != not compatible with types contract C and contract C. No arithmetic or comparison operations are allowed on contract types. Consider converting to "address".
// TypeError 2271: (146-155): Operator >= not compatible with types contract C and contract C. No arithmetic or comparison operations are allowed on contract types. Consider converting to "address".
// TypeError 2271: (167-176): Operator <= not compatible with types contract C and contract C. No arithmetic or comparison operations are allowed on contract types. Consider converting to "address".
// TypeError 2271: (188-196): Operator < not compatible with types contract C and contract C. No arithmetic or comparison operations are allowed on contract types. Consider converting to "address".
// TypeError 2271: (208-216): Operator > not compatible with types contract C and contract C. No arithmetic or comparison operations are allowed on contract types. Consider converting to "address".
