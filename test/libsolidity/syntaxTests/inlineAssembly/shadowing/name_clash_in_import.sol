==== Source: a ====
contract A
{
	uint constant a = 42;
}
==== Source: b ====
import {A as b} from "a";
contract B {
    function f() public pure {
        assembly {
            let b := 3
            let b.a := 4
        }
    }
}
// ----
// DeclarationError: (b:105-106): This declaration shadows a declaration outside the inline assembly block.
// DeclarationError: (b:128-131): The prefix of this declaration conflicts with a declaration outside the inline assembly block.
