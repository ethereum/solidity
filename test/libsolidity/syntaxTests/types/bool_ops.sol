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
// TypeError: (231-236): Operator > not compatible with types bool and bool
// TypeError: (250-255): Operator < not compatible with types bool and bool
// TypeError: (269-275): Operator >= not compatible with types bool and bool
// TypeError: (289-295): Operator <= not compatible with types bool and bool
// TypeError: (309-314): Operator & not compatible with types bool and bool
// TypeError: (328-333): Operator | not compatible with types bool and bool
// TypeError: (347-352): Operator ^ not compatible with types bool and bool
// TypeError: (366-368): Unary operator ~ cannot be applied to type bool
// TypeError: (382-384): Unary operator ~ cannot be applied to type bool
// TypeError: (398-403): Operator + not compatible with types bool and bool
// TypeError: (417-422): Operator - not compatible with types bool and bool
// TypeError: (436-438): Unary operator - cannot be applied to type bool
// TypeError: (452-454): Unary operator - cannot be applied to type bool
// TypeError: (468-473): Operator * not compatible with types bool and bool
// TypeError: (487-492): Operator / not compatible with types bool and bool
// TypeError: (506-512): Operator ** not compatible with types bool and bool
// TypeError: (526-531): Operator % not compatible with types bool and bool
// TypeError: (545-551): Operator << not compatible with types bool and bool
// TypeError: (565-571): Operator >> not compatible with types bool and bool
