contract C {
    function f(bool a, bool b) public pure {
        bool c;
        // OK
        c = !a;
        c = !b;
        c = a == b;
        c = a != b;
        c = a || b;
        c = a && b;

        // Not OK
        c = a > b;
        c = a < b;
        c = a >= b;
        c = a <= b;
        c = a & b;
        c = a | b;
        c = a ^ b;
        c = ~a;
        c = ~b;
        c = a + b;
        c = a - b;
        c = -a;
        c = -b;
        c = a * b;
        c = a / b;
        c = a ** b;
        c = a % b;
        c = a << b;
        c = a >> b;
    }
}
// ----
// TypeError 2271: (231-236='a > b'): Operator > not compatible with types bool and bool
// TypeError 2271: (250-255='a < b'): Operator < not compatible with types bool and bool
// TypeError 2271: (269-275='a >= b'): Operator >= not compatible with types bool and bool
// TypeError 2271: (289-295='a <= b'): Operator <= not compatible with types bool and bool
// TypeError 2271: (309-314='a & b'): Operator & not compatible with types bool and bool
// TypeError 2271: (328-333='a | b'): Operator | not compatible with types bool and bool
// TypeError 2271: (347-352='a ^ b'): Operator ^ not compatible with types bool and bool
// TypeError 4907: (366-368='~a'): Unary operator ~ cannot be applied to type bool
// TypeError 4907: (382-384='~b'): Unary operator ~ cannot be applied to type bool
// TypeError 2271: (398-403='a + b'): Operator + not compatible with types bool and bool
// TypeError 2271: (417-422='a - b'): Operator - not compatible with types bool and bool
// TypeError 4907: (436-438='-a'): Unary operator - cannot be applied to type bool
// TypeError 4907: (452-454='-b'): Unary operator - cannot be applied to type bool
// TypeError 2271: (468-473='a * b'): Operator * not compatible with types bool and bool
// TypeError 2271: (487-492='a / b'): Operator / not compatible with types bool and bool
// TypeError 2271: (506-512='a ** b'): Operator ** not compatible with types bool and bool
// TypeError 2271: (526-531='a % b'): Operator % not compatible with types bool and bool
// TypeError 2271: (545-551='a << b'): Operator << not compatible with types bool and bool
// TypeError 2271: (565-571='a >> b'): Operator >> not compatible with types bool and bool
