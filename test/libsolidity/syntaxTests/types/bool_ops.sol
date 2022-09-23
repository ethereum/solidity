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
// TypeError 2271: (231-236): Built-in binary operator > cannot be applied to types bool and bool.
// TypeError 2271: (250-255): Built-in binary operator < cannot be applied to types bool and bool.
// TypeError 2271: (269-275): Built-in binary operator >= cannot be applied to types bool and bool.
// TypeError 2271: (289-295): Built-in binary operator <= cannot be applied to types bool and bool.
// TypeError 2271: (309-314): Built-in binary operator & cannot be applied to types bool and bool.
// TypeError 2271: (328-333): Built-in binary operator | cannot be applied to types bool and bool.
// TypeError 2271: (347-352): Built-in binary operator ^ cannot be applied to types bool and bool.
// TypeError 4907: (366-368): Built-in unary operator ~ cannot be applied to type bool.
// TypeError 4907: (382-384): Built-in unary operator ~ cannot be applied to type bool.
// TypeError 2271: (398-403): Built-in binary operator + cannot be applied to types bool and bool.
// TypeError 2271: (417-422): Built-in binary operator - cannot be applied to types bool and bool.
// TypeError 4907: (436-438): Built-in unary operator - cannot be applied to type bool.
// TypeError 4907: (452-454): Built-in unary operator - cannot be applied to type bool.
// TypeError 2271: (468-473): Built-in binary operator * cannot be applied to types bool and bool.
// TypeError 2271: (487-492): Built-in binary operator / cannot be applied to types bool and bool.
// TypeError 2271: (506-512): Built-in binary operator ** cannot be applied to types bool and bool.
// TypeError 2271: (526-531): Built-in binary operator % cannot be applied to types bool and bool.
// TypeError 2271: (545-551): Built-in binary operator << cannot be applied to types bool and bool.
// TypeError 2271: (565-571): Built-in binary operator >> cannot be applied to types bool and bool.
