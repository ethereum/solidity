.. index:: Bugs

.. _known_bugs:

##################
List of Known Bugs
##################

Below, you can find a JSON-formatted list of all known security-relevant bugs in the
Solidity compiler. The file itself is hosted in the `Github repository
<https://github.com/ethereum/solidity/blob/develop/docs/bugs.json>`_.
The list stretches back as far as version 0.3.0, bugs known to be present only
in previous versions are not listed. The JSON file is an array of objects, one for
each bug, with the following keys:

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
    Severity of the bug: low, medium, high. Takes into account
    discoverability in contract tests, likelihood of occurrence and
    potential damage by exploits.
conditions
    Conditions that have to be met to trigger the bug. Currently, this
    is an object that can contain a boolean value ``optimizer``, which
    means that the optimizer has to be switched on to enable the bug.
    If no conditions are given, assume that the bug is present.

.. literalinclude:: bugs.json
   :language: js
