<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Introduction](#introduction)
- [_GetToken()_ doesn't indicate success/failure](#gettoken-doesnt-indicate-successfailure)
- [Out-of-sync closers](#out-of-sync-closers)
- [Dual levels of Error verbosity](#dual-levels-of-error-verbosity)
- [Filtering Cascaded Errors](#filtering-cascaded-errors)
- [Documenting how Error recover works](#documenting-how-error-recover-works)

<!-- markdown-toc end -->

Introduction
------------

This is a to-do list and a place for discussion of how to improve error handling, and may guide current development.

GetToken() doesn't indicate success/failure
-------------------------------------------
Routines that call _GetToken()_, like _expectIdentifierToken()_ can't take corrective action. Here is an example.

In

```
    function f() public pure {
        address payable;
	//             ^ identifier expected here
    }
```

there is a semicolon where an identifier is expected. _expectIdentifierToken()_ will gobble this up leaving no semicolon for _Statement_ to synchronize to. for _expectIdentifierToken()_ to be smarter, it would need to get status back from _GetToken()_.

Out-of-sync closers
-------------------

Steve C Johnson recovery is a coarse kind of recovery, and here it can cause cascaded effects as a result of skipping over one kind of terminator while looking for another.
Right now, there is only one synchronizing token at a time for a rule,
either `;` or `}`.

Here is demonstration of the cascaded effect problem.

Consider this code:

```
contract Error8 {
  function buggy(int256 a) internal view returns (uint256) {
    if (a <= 0) {
      a += // no RHS
    }
    return a;
  }
}
```

At the point of `a +=` we are looking for a "primary expression" inside _Statement_. _Statement_ recovers by looking for a semicolon.
By passing over the other _Block_ boundary in recovery, we are now mismatched in nesting levels.

In effect, with current error recovery the program is seen as:

```
contract Error8 {
  function buggy(int256 a) internal view returns (uint256) {
    if (a <= 0) {
      a += ... ; // skips over }
  }
}
```

Additional Error Rules
----------------------

* ParameterList ')'
* Doc strings
* other things with '('

A remedy would be to note additional higher-level terminators, here right brace.
Better might be to stop if either semicolon or right brace is seen first. Seeing a right brace has the effect of closing looking for a _Statement_  and then allowing _Block_ to find its expected right brace. Nesting levels would no longer be mismatched and the subsequent `return a;` would be handled properly.

Dual levels of Error verbosity
------------------------------

IDEs and front-end tools that call solidity would prefer to get error messages in some structured way, like JSON. Also they may be able to use additional information that a command-line user is not interested in.

Filtering Cascaded Errors
-------------------------

Because there will always be cascaded errors, there is benefit to looking for this kind of effect and removing messages from incomplete recovery when errors are reported in command-line mode. Or we can keep things the way they are now by default, and have an option to turn on more pervasive error recovery.

Documenting how Error recovery works
------------------------------------

A first cut of this was done, but is currently omitted since that is a side track right now.
