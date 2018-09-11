.. index:: Bugs

.. _known_bugs:

##################
List of Known Bugs
##################

Below, you can find a JSON-formatted list of some of the known security-relevant bugs in the
Solidity compiler. The file itself is hosted in the `Github repository
<https://github.com/ethereum/solidity/blob/develop/docs/bugs.json>`_.
The list stretches back as far as version 0.3.0, bugs known to be present only
in versions preceding that are not listed.

There is another file called `bugs_by_version.json
<https://github.com/ethereum/solidity/blob/develop/docs/bugs_by_version.json>`_,
which can be used to check which bugs affect a specific version of the compiler.

Contract source verification tools and also other tools interacting with
contracts should consult this list according to the following criteria:

 - It is mildly suspicious if a contract was compiled with a nightly
   compiler version instead of a released version. This list does not keep
   track of unreleased or nightly versions.
 - It is also mildly suspicious if a contract was compiled with a version that was
   not the most recent at the time the contract was created. For contracts
   created from other contracts, you have to follow the creation chain
   back to a transaction and use the date of that transaction as creation date.
 - It is highly suspicious if a contract was compiled with a compiler that
   contains a known bug and the contract was created at a time where a newer
   compiler version containing a fix was already released.

The JSON file of known bugs below is an array of objects, one for each bug,
with the following keys:

name
    Unique name given to the bug
summary
    Short description of the bug
description
    Detailed description of the bug
link
    URL of a website with more detailed information, optional
introduced
    The first published compiler version that contained the bug, optional
fixed
    The first published compiler version that did not contain the bug anymore
publish
    The date at which the bug became known publicly, optional
severity
    Severity of the bug: very low, low, medium, high. Takes into account
    discoverability in contract tests, likelihood of occurrence and
    potential damage by exploits.
conditions
    Conditions that have to be met to trigger the bug. Currently, this
    is an object that can contain a boolean value ``optimizer``, which
    means that the optimizer has to be switched on to enable the bug.
    If no conditions are given, assume that the bug is present.
check
    This field contains different checks that report whether the smart contract
    contains the bug or not. The first type of check are Javascript regular
    expressions that are to be matched against the source code ("source-regex")
    if the bug is present.  If there is no match, then the bug is very likely
    not present. If there is a match, the bug might be present.  For improved
    accuracy, the checks should be applied to the source code after stripping
    comments.
    The second type of check are patterns to be checked on the compact AST of
    the Solidity program ("ast-compact-json-path"). The specified search query
    is a `JsonPath <https://github.com/json-path/JsonPath>`_ expression.
    If at least one path of the Solidity AST matches the query, the bug is
    likely present.

.. literalinclude:: bugs.json
   :language: js
