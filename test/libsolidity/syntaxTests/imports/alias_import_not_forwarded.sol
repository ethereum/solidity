==== Source: dummy ====
contract Dummy { string public constant FOO = "FOO"; }
==== Source: hasAlias ====
import {Dummy as AliasedDummy} from "dummy";
==== Source: Main ====
import {AliasedDummy} from "hasAlias";
contract TestAlias is AliasedDummy {}
// ----
