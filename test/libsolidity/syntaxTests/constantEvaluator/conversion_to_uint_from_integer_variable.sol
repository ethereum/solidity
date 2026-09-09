uint constant CONST = uint(42);
uint constant CONST2 = uint(64);
int constant SIGNED_POSITIVE = 2;
int constant SIGNED_NEGATIVE = -1;

contract C layout at CONST {
    int[uint(CONST2)] a;
    int[uint(SIGNED_POSITIVE)] b;
    int[uint(CONST + CONST2)] c;
    int[uint(CONST / 2)] d;
    int[uint(SIGNED_NEGATIVE) / 2**255] e;
}
// ----
