==== Source: a ====
contract A {}
==== Source: b ====
import {A as AliasedA, } from "a";
// ----
