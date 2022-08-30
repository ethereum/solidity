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
// TypeError 2271: (231-236): Binary operator > not compatible with types bool and bool.
// TypeError 2271: (250-255): Binary operator < not compatible with types bool and bool.
// TypeError 2271: (269-275): Binary operator >= not compatible with types bool and bool.
// TypeError 2271: (289-295): Binary operator <= not compatible with types bool and bool.
// TypeError 2271: (309-314): Binary operator & not compatible with types bool and bool.
// TypeError 2271: (328-333): Binary operator | not compatible with types bool and bool.
// TypeError 2271: (347-352): Binary operator ^ not compatible with types bool and bool.
// TypeError 4907: (366-368): Unary operator ~ cannot be applied to type bool.
// TypeError 4907: (382-384): Unary operator ~ cannot be applied to type bool.
// TypeError 2271: (398-403): Binary operator + not compatible with types bool and bool.
// TypeError 2271: (417-422): Binary operator - not compatible with types bool and bool.
// TypeError 4907: (436-438): Unary operator - cannot be applied to type bool.
// TypeError 4907: (452-454): Unary operator - cannot be applied to type bool.
// TypeError 2271: (468-473): Binary operator * not compatible with types bool and bool.
// TypeError 2271: (487-492): Binary operator / not compatible with types bool and bool.
// TypeError 2271: (506-512): Binary operator ** not compatible with types bool and bool.
// TypeError 2271: (526-531): Binary operator % not compatible with types bool and bool.
// TypeError 2271: (545-551): Binary operator << not compatible with types bool and bool.
// TypeError 2271: (565-571): Binary operator >> not compatible with types bool and bool.
