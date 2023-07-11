# PR Review Checklist
The Solidity compiler is a critical piece of infrastructure in the Ethereum ecosystem.
For this reason, our review process is quite strict and all PRs have to fulfill certain quality
expectations and guidelines.
The list below is meant to reduce the workload on the core team by helping contributors self-identify
and solve common issues before they are pointed out in the review.
It is also meant to serve as a final checklist for reviewers to go through before approving a PR.

## Before You Submit a PR
- [ ] **Do you have any other open PRs?**
    Work on a PR is not done until it is merged or closed.
    Our reviewing capacity is limited, so we require that external contributors work on **no more than one PR at a time**.
    - If your PR is not getting reviewed, feel free to bring it to our attention on the [#solidity-dev](https://gitter.im/ethereum/solidity-dev) channel.
    - Unless they were requested, we are going to close any excess PRs, leaving only the earliest one open.
        You may reopen them, one at a time, when your current PR is done.
- [ ] **Is the issue ready to be worked on?**
    - If the issue does not have a desirability label (`nice to have`, `should have`,
        `must have eventually`, `must have`, `roadmap`) we have not yet decided whether to implement it.
    - If the issue has the `needs design` label, we have not yet decided how it should be implemented.
    - `good first issue candidate` means that the issue will potentially be a `good first issue`
        eventually but at the moment it is not yet ready to be worked on.
- [ ] **Is this a breaking change?** Breaking changes should be based on the `breaking` branch rather than on the `develop` branch.
- [ ] **Does the PR actually address the issue?**
    - [ ] Mention the issue number in the PR description.
        If the PR solves it completely, use the `Fixes #<issue number>` form so that Github will close the issue automatically.
    - [ ] Do not include the issue number in the PR title, branch name or commit description.
- [ ] When submitting a PR from a fork **create a branch and give it a descriptive name.**
    E.g. `fix-array-abi-encoding-bug`.
    Do not submit PRs directly from the `develop` branch of your fork since it makes rebasing and fetching new changes harder.
- [ ] **Does the PR depend on other PRs?**
    - [ ] If the PR has dependencies, mention them in bold in the description.
    - [ ] Avoid basing PRs from forks on branches other than `develop` or `breaking` because
        GitHub closes them when the base branch gets merged.
        Do this only for PRs created directly in the main repo.
- [ ] **Does the PR update test expectations to match the modified code?** If not, your PR will not pass some of the `_soltest_`,  jobs in CI.
    In many cases the expectations can be updated automatically:
    - `cmdlineTests.sh --update` for command-line tests.
    - `isoltest --enforce-gas-cost --accept-updates` for soltest-based tests.
        - If your PR affects gas costs, an extra run of `isoltest --enforce-gas-cost --optimize --accept-updates` is needed to update gas expectations with optimizer enabled.
    - Review updated files before committing them.
        **Are expectations correct and do updated tests still serve their purpose?**

## Abandoned PRs
- [ ] **Is the submitter still responsive?**
    - If the PR had no activity from the submitter in the last 2 weeks (despite receiving reviews and our prompts) we consider it abandoned.
- [ ] **Is the abandoned PR easy to finish or relevant?**
    - Apply the `takeover` label if the PR can be finished without significant effort or is something that actually needs to be done right now.
        Otherwise close it.
        It can still be taken over later or reopened by the submitter but until then we should not be getting sidetracked by it.

## Light Review
Before an in-depth review, it is recommended to give new PRs a quick, superficial review, which
is not meant to provide complete and detailed feedback, but instead give the submitter a rough idea
if the PR is even on the right track and let them solve the obvious problems on their own.

Light review should focus on the following three areas:
- [ ] **Are there any obvious mistakes?** Style issues, bad practices, easy to identify bugs, etc.
- [ ] **Is there anything missing?** Tests (of the right kind), documentation, etc. Does it address the whole issue?
- [ ] **Is it the right solution?** Are there better ways to do this? Is the change even necessary?

If the answers above are "Yes, Yes, No", thank the contributor for their effort and **close the PR**.

## Coding Style and Good Practices
- [ ] Does the PR follow our [coding style](CODING_STYLE.md)?

### Reliability
- [ ] **Use assertions liberally.** If you are certain your assumption will not be broken, prove it with `solAssert()`.
- [ ] **Validate inputs and handle errors**. Note that assertions are **not** validation.

### Readability
- [ ] **Choose good names.**
    - [ ] Is the name straightforward to understand?
        Do you feel the need to jump back to the definition and remind yourself what it was whenever you see it?
    - [ ] Is the name unambiguous in the context where it is used?
    - [ ] Avoid abbreviations.
- [ ] **Source files, classes and public functions should have docstrings.**
- [ ] **Avoid code duplication.** But not fanatically. Minimal amounts of duplication are acceptable if it aids readability.
- [ ] **Do not leave dead or commented-out code behind.** You can still see old code in history.
      If you really have a good reason to do it, always leave a comment explaining what it is and why it is there.
- [ ] **Mark hacks as such.** If you have to leave behind a temporary workaround, make
    sure to include a comment that explains why and in what circumstances it can be removed.
    Preferably link to an issue you reported upstream.
- [ ] **Avoid obvious comments.**
- [ ] **Do include comments when the reader may need extra context to understand the code.**

### Commits and PRs
- [ ] **Avoid hiding functional changes inside refactors.**
    E.g. when fixing a small bug, or changing user-visible behavior, put the change in a separate commit.
    Do not mix it with another change that renames things or reformats the code around, making the fix itself hard to identify.
- [ ] **Whenever possible, split off refactors or unrelated changes into separate PRs.**
    Smaller PRs are easier and quicker to review.
    Splitting off refactors helps focus on the main point of the PR.

### Common Pitfalls
The following points are all covered by the coding style but come up so often that it is worth singling them out here:
- [ ] **Always initialize value types in the definition,** even if you are sure you will assign them later.
- [ ] **Use "east const" style.** I.e. `T const*`, not `const T *`.
- [ ] **Keep indentation consistent.** See our [`.editorconfig`](.editorconfig).
    - [ ] Tabs for C++. But use them for indentation only. Any whitespace later on the line must consist of spaces.
    - [ ] 4 spaces for most other file types.
- [ ] **Use `auto` sparingly.** Only use it when the actual type is very long and complicated or when it is
    already used elsewhere in the same expression.
- [ ] **Indent braces and parentheses in a way that makes nesting clear.**
- [ ] **Use `using namespace` only in `.cpp` files.** Use it for `std` and our own modules.
    Avoid unnecessary `std::` prefix in `.cpp` files (except for `std::move` and `std::forward`).
- [ ] **Use range-based loops and destructuring.**
- [ ] **Include any headers you use directly,** even if they are implicitly included through other headers.

## Documentation
- [ ] **Does the PR update relevant documentation?**

### Documentation Style and Good Practices
- [ ] **Use double backticks in RST (``` ``x`` ```). Prefer single backticks in Markdown (`` `x` ``),**
    but note that double backticks are valid too and we use them in some cases for legacy reasons.
- [ ] **Always start a new sentence on a new line.**
    This way you do not have to rewrap the surrounding text when you rewrite the sentence.
    This also makes changes actually easier to spot in the diff.

## Testing

### What to Test
- [ ] **Is newly added code adequately covered by tests?** Have you considered all the important corner cases?
- If it is a bugfix:
    - [ ] **The PR must include tests that reproduce the bug.**
    - [ ] **Are there gaps in test coverage of the buggy feature?** Fill them by adding more tests.
    - [ ] **Try to break it.** Can you of any similar features that could also be buggy?
        Play with the repro and include prominent variants as separate test cases, even if they don't trigger a bug.
- [ ] **Positive cases (code that compiles) should have a semantic test.**
- [ ] **Negative cases (code with compilation errors) should have a syntax test.**
- [ ] **Avoid mixing positive and negative cases in the same syntax test.**
    If the test produces an error, we stop at the analysis stage and we will not detect
    problems that only occur in code generation, optimizer or assembler.
    - [ ] If you have to do it, at least mark positive cases inside the file with a short comment.
        - This way, when the test is updated, it is easier to verify that these cases still do not trigger an error.
- [ ] New syntax: **does it have an [`ASTJSON`](test/libsolidity/ASTJSON/) test?**
- [ ] New CLI or StandardJSON option:
    - [ ] **Does it have a [command-line test](test/cmdlineTests/)?**
    - [ ] **Is the option listed for every input mode in [`CommandLineParser` tests](test/solc/CommandLineParser.cpp)?**
- [ ] **Did you consider interactions with other language features?**
    - [ ] Are all types covered? Structs? Enums? Contracts/libraries/interfaces? User-defined value types?
        Value types: integers, fixed bytes, `address`, `address payable`, `bool`? Function pointers?
        Static and dynamic arrays? `string` and `bytes`? Mappings?
        Values of types that cannot be named: literals, tuples, array slices, storage references?
    - [ ] If it accepts a function, does it also accept an event or an error? These have function types but are not functions.
    - [ ] If it affects free functions, what about internal library functions?
    - [ ] Attached library functions? Functions attached with `using for`?
    - [ ] Possible combinations of `storage`, `memory`, `calldata`, `immutable`, `constant`?
        Remember that internal functions can take `storage` arguments.
    - [ ] Does it work at construction time as well? What if you store it at construction time and read after deployment?
    - [ ] What about importing it from a different module or inheriting it?
    - [ ] Have you tested it with the ternary operator?

### Test Style and Good Practices
- [ ] **Make test case file names long and specific enough** so that it is easy to guess what is inside.
    When checking if we have the case already covered the name is usually the only clue we see.
    - [ ] Place them in the right subdirectory.
    - [ ] **Avoid simply appending numbers to the name to distinguish similar cases.**
        Coming up with good names is hard but figuring out if any of hundreds of tests with names that
        match your search actually fits your case is even harder.
- [ ] **Do not include version pragma and the SPDX comment in semantic and syntax test cases**.
    In other test types include them if necessary to suppress warnings.
- [ ] **If you have to use a version pragma, avoid hard-coding version.** Use `pragma solidity *`.
- [ ] **When writing StandardJSON command-line tests, use `urls` instead of `content`** and put
    the Solidity or Yul code in a separate file.

## Compiler-specific
- [ ] **Are error messages sensible and understandable to users?**
- [ ] **Are error codes consistent?**
    - [ ] Avoid randomly changing or reassigning error codes.
    - [ ] Avoid defining separate codes for trivial variants of the same issue.
        Make it easy for tools to consistently refer to the same error with the same code.
- [ ] **Error messages should end with a full stop.**
- [ ] **Prefer Ranges v3 to Boost where possible.**

## Take a Step Back
- [ ] **Do you fully understand what the PR does and why?**
- [ ] **Are you confident that the code works and does not break unrelated functionality?**
- [ ] **Is this a reasonable way to achieve the goal stated in the issue?**
- [ ] **Is the code simple?** Does the PR achieve its objective at the cost of significant
    complexity that may be a source of future bugs?
- [ ] **Is the code efficient?** Does the PR introduce any major performance bottlenecks?
- [ ] **Does the PR introduce any breaking changes beyond what was agreed in the issue?**

## Final Checks Before Merging
- [ ] **Is the PR rebased on top of the `develop` branch** (or `breaking` if it is a breaking change)?
- [ ] **Did all CI checks pass?**
    - Note that we have a few jobs that tend to randomly fail due to outside factors, especially external tests (with `_ext_` in the name).
        If these fail, rebase on latest `develop` (or `breaking`) and try rerunning them.
        Note also that not all of these checks are required for the PR to be merged.
- [ ] If the change is visible to users, **does the PR have a [changelog](Changelog.md) entry?**
    - [ ] Is the changelog entry in the right section?
        Make sure to move it up if there was a release recently.
- [ ] **Is commit history simple and understandable?**
    - [ ] Each commit should be a self-contained, logical step leading the goal of the PR, without going back and forth.
        In particular, review fixups should be squashed into the commits they fix.
    - [ ] Do not include any merge commits in your branch. Please use rebase to keep up to date with the base branch.
- [ ] **Is the PR properly labeled?**
    - Use `external contribution` label to mark PRs not coming from the core team.
    - If the PR depends on other PRs, use `has dependencies` and set the base branch accordingly.
    - Labels like `documentation` or `optimizer` are helpful for filtering PRs.
