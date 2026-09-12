# Security Policy

The Solidity team and community take all security bugs in Solidity seriously.
We appreciate your efforts and responsible disclosure and will make every effort to acknowledge your contributions.

## Scope

Bugs in the Solidity repository are in scope.
Bugs in third-party dependencies e.g., nlohmann-json, boost etc. are not in scope unless they result in a Solidity specific bug.

Only bugs that have a demonstrable security impact on smart contracts are in scope.
For example, a Solidity program whose optimization is incorrect (e.g., leads to an incorrect output) qualifies as a security bug.
Please note that the [rules][2] of the [Ethereum bounty program][1] have precedence over this security policy.

## Before You Report

Please check that what you found is not
- intentional,
- documented,
- explicitly not guaranteed by the language,
- contained in the [summary of known security vulnerabilities][3], or
- already reported in the [public issue tracker][4].

Furthermore, please check whether it is one of the [frequently reported non-bugs][5] and, if it resembles one of them, explain in your report why you still consider it a compiler bug.

## Supported Versions

As a general rule, only the latest release gets security updates.
Exceptions may be made when the current breaking release is relatively new, e.g. less than three months old.
If you are reporting a bug, please state clearly the Solidity version(s) it affects.

Example 1: Assuming the current release is `0.6.3` and a security bug has been found in it that affects both `0.5.x` and `0.6.x` trees, we may not only patch `0.6.3` (the bug-fix release numbered `0.6.4`) but `0.5.x` as well (the bug-fix release numbered `0.5.(x+1)`).

Example 2: Assuming the current release is `0.6.25` and a security bug has been found in it, we may only patch `0.6.25` (in the bug-fix release numbered `0.6.26`) even if the bug affects a previous tree such as `0.5.x`.

## Reporting a Vulnerability

To report a vulnerability, please follow the instructions stated in the [Ethereum bounty program][1].

In the bug report, please include all details necessary to reproduce the vulnerability such as:

- Input program that triggers the bug
- Compiler version affected
- Target EVM version
- Framework/IDE if applicable
- EVM execution environment/client if applicable
- Operating system

Please include steps to reproduce the bug you have found in as much detail as possible.

Once we have received your bug report, we will try to reproduce it and provide a more detailed response.
Once the reported bug has been successfully reproduced, the Solidity team will work on a fix.

The Solidity team maintains the following JSON-formatted lists of patched security vulnerabilities:

- [Summary of known security vulnerabilities][3]
- [List of security vulnerabilities affecting a specific version of the compiler][4].


[1]: https://bounty.ethereum.org/
[2]: https://bounty.ethereum.org/#rules
[3]: https://docs.soliditylang.org/en/develop/bugs.html
[4]: https://github.com/argotorg/solidity/issues
[5]: https://docs.soliditylang.org/en/develop/frequently-reported-non-bugs.html
