==== Source: A ====
type MyInt is int;
type MyAddress is address;
==== Source: B ====
import {MyAddress as OurAddress} from "A";
contract A {
    function f(address x) external view returns(MyAddress) { return MyAddress.wrap(x); }
}
// ----
// DeclarationError 7920: (B:104-113): Identifier not found or not unique.
