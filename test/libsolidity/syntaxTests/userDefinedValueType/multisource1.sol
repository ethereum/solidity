==== Source: A ====
type MyInt is int;
type MyAddress is address;
==== Source: B ====
import {MyAddress as OurAddress} from "A";
contract A {
    function f(int x) external view returns(MyInt) { return MyInt.wrap(x); }
}
// ----
// DeclarationError 7920: (B:100-105): Identifier not found or not unique.
