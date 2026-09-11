# Checklist for making a Solidity release

## Requirements
- GitHub account with access to [solidity](https://github.com/argotorg/solidity), [solc-js](https://github.com/argotorg/solc-js),
      [solc-bin](https://github.com/argotorg/solc-bin), [solidity-website](https://github.com/argotorg/solidity-website).
- Personal Access Token (PAT) with `write:packages` scope to access Github's container registry.
    You can generate one by visiting https://github.com/settings/tokens/new?scopes=write:packages.
- Ubuntu/Debian dependencies of the Docker script: `docker-buildx`.
- [npm Registry](https://www.npmjs.com) account added as a collaborator for the [`solc` package](https://www.npmjs.com/package/solc).
- Access to the [solidity_lang Twitter account](https://twitter.com/solidity_lang).
- [Reddit](https://www.reddit.com) account that is at least 10 days old with a minimum of 20 comment karma (`/r/ethereum` requirements).

## Full release

### Pre-flight checks
At least a day before the release:
- [ ] Run `make linkcheck` from within `docs/` and fix any broken links it finds.
      Ignore false positives caused by `href` anchors and dummy links not meant to work.
      **Note**: In order to run the link check, make sure you've built the docs first via `docs.sh`.
- [ ] Double-check that [the most recent docs builds at readthedocs](https://readthedocs.org/projects/solidity/builds/) succeeded.
- [ ] Make sure that all merged PRs that should have changelog entries do have them.
- [ ] Rerun CI on the top commits of main branches in all repositories that do not have daily activity by creating a test branch or PR:
     - [ ] `solc-js`
     - [ ] `solc-bin` (make sure the bytecode comparison check did run)
- [ ] Verify that the release tarball of `solc-js` works.
      Bump version locally, add `soljson.js` from CI, build it, compare the file structure with the previous version, install it locally and try to use it.
- [ ] Review [Learning from Past Releases](https://notes.argot.org/@solidity-release-mistakes) to make sure you don't repeat the same mistakes.

### Drafts
At least a day before the release:
- [ ] Create a draft PR to sort the changelog.
- [ ] Create draft PRs to bump version in `solidity` and `solc-js`.
      **Note**: The `solc-js` PR won't pass CI checks yet because it depends on the soljson binary from `solc-bin`.
- [ ] Create a draft of the release on github.
- [ ] Create a draft PR to update soliditylang.org.
- [ ] Create drafts of blog posts.
- [ ] Prepare drafts of Twitter, Reddit and Solidity Forum announcements.

### Website/Blog Updates
- [ ] Create a post on [solidity-website](https://github.com/argotorg/solidity-website/tree/main/src/posts) in the `Releases` category and explain some of the new features or concepts.
- [ ] Create a post on [solidity-website](https://github.com/argotorg/solidity-website/tree/main/src/posts) in the `Security Alerts` category in case of important bug(s).
- [ ] Update the release information section [in the source of soliditylang.org](https://github.com/argotorg/solidity-website/blob/main/src/pages/index.tsx).
- [ ] Get the PR(s) for the above reviewed and approved **before the release starts**, but don't merge them yet.

### Changelog
- [ ] Ensure that all changelog entries are correctly classified as language or compiler features.
- [ ] Ensure that every important bug has a `bugs.json` entry **and** a changelog entry under "Important Bugfixes" (not "Bugfixes").
    Verify that every bug list entry added in this release has a sequential `uid`, uses the new version in the `fixed` field and has a blog `link` matching the release date.
- [ ] Sort the changelog entries alphabetically and correct any errors you notice. Commit it.
- [ ] Update the changelog to include a release date.
- [ ] Run `scripts/update_bugs_by_version.py` to regenerate `bugs_by_version.json` from the changelog and `bugs.json`.
      Make sure that the resulting `bugs_by_version.json` has a new, empty entry for the new version.
- [ ] Commit changes, create a pull request and wait for the tests. Then merge it.
- [ ] Copy the changelog into the release blog post.

### Create the Release
- [ ] Create a [release on GitHub](https://github.com/argotorg/solidity/releases/new).
      Set the target to the `develop` branch and the tag to the new version, e.g. `v0.8.5`.
      Include the following warning: `**The release is still in progress. You may see broken links and binaries may not yet be available from all sources.**`.
      Do not publish it yet - click the `Save draft` button instead.
- [ ] Thank voluntary contributors in the GitHub release notes.
      Use `scripts/list_contributors.sh v<previous version>` to get initial list of names.
      Remove different variants of the same name manually before using the output.
- [ ] Check that all tests on the latest commit on `develop` are green.
- [ ] Click the `Publish release` button on the release page, creating the tag.
      **Important: Must not be done before all the PRs, including changelog cleanup and date, are merged.**
- [ ] Wait for the CI runs on the tag itself.

### Upload Release Artifacts and Publish Binaries
- [ ] Take the source tarball (`solidity_x.x.x.tar.gz`) from `c_source_tarball` run of the tagged commit on Circle CI and upload it to the release page.
- [ ] Take the `github-binaries.tar` tarball from `c_release_binaries` run of the tagged commit on Circle CI and add all binaries from it to the release page.
      Make sure it contains five binaries: `solc-windows.exe`, `solc-macos`, `solc-static-linux`, `solc-static-linux-arm`, and `soljson.js`.
- [ ] Take the `solc-bin-binaries.tar` tarball from `c_release_binaries` run of the tagged commit on Circle CI and add all binaries from it to solc-bin.
- [ ] Run `npm install` if you've got a clean checkout of the solc-bin repo.
- [ ] Run `npm run update -- --reuse-hashes` in `solc-bin` and verify that the script has updated `list.js`, `list.txt` and `list.json` files correctly and that symlinks to the new release have been added in `solc-bin/wasm/` and `solc-bin/emscripten-wasm32/`.
- [ ] Create a pull request in solc-bin and merge.

### Homebrew and MacOS
- [ ] Verify that the Homebrew bot picked up on the new release and a PR updating the [`solidity` formula](https://github.com/Homebrew/homebrew-core/blob/master/Formula/s/solidity.rb) has been [automatically submitted](https://github.com/Homebrew/homebrew-core/pulls?q=is:pr+solidity) and merged.


### Docker
- [ ] Make sure `docker-buildx` is installed.
- [ ] Run `echo $GHCR_TOKEN | docker login ghcr.io --username $GH_USERNAME --password-stdin` where `$GH_USERNAME` is your GitHub username and `$GHCR_TOKEN` is a PAT with `write:packages` scope.
- [ ] Run `./scripts/docker_deploy_manual.sh v$VERSION`.

### Release solc-js
- [ ] Wait until solc-bin was properly deployed. You can test this via remix - a test run through remix is advisable anyway.
- [ ] Increment the version number, create a pull request for that, merge it after tests succeeded.
- [ ] Create a tag using `git tag --annotate v$VERSION` and push it with `git push --tags`.
- [ ] Wait for the CI runs on the tag itself.
- [ ] Take the `solc-x.y.z.tgz` artifact from `build-package` run on the tagged commit on Circle CI.
      Inspect the tarball to ensure that it contains an up-to-date compiler binary (`soljson.js`).
- [ ] Run `npm publish solc-x.y.z.tgz` to publish the newly created tarball.

### Documentation
- [ ] Make sure the documentation for the new release has been published successfully.
      Go to the [documentation status page at ReadTheDocs](https://readthedocs.org/projects/solidity/) and verify that the new version is listed, works and is marked as default.
- [ ] Remove "still in progress" warning from the [release notes](https://github.com/argotorg/solidity/releases).

### Comms
- [ ] Merge the blog posts and website updates prepared for the release.
- [ ] Verify that the link to the blog post in [release notes](https://github.com/argotorg/solidity/releases) is not broken.
- [ ] Announce on [Twitter](https://twitter.com/solidity_lang), including links to the release and the blog post.
- [ ] Announce on [Fosstodon](https://fosstodon.org/@solidity/), including links to the release and the blog post.
- [ ] Share the announcement on Reddit in [`/r/ethdev`](https://reddit.com/r/ethdev/), cross-posted to [`/r/ethereum`](https://reddit.com/r/ethereum/).
- [ ] Share the announcement on the [Solidity forum](https://forum.soliditylang.org) in the `Announcements` category.
- [ ] Share the announcement on [`#solidity` channel on Matrix](https://matrix.to/#/#ethereum_solidity:gitter.im).
- [ ] Share the announcement on [`#solc-tooling`](https://matrix.to/#/#solc-tooling:matrix.org).

### Post-release
- [ ] Create a commit to increase the version number on `develop` in `CMakeLists.txt` and add a new skeleton changelog entry.
- [ ] If anything went wrong this time, mention it in [Learning from Past Releases](https://notes.argot.org/@solidity-release-mistakes).
- [ ] Bump vendored dependencies.
- [ ] Lean back, wait for bug reports and repeat from step 1 :).

## Prerelease
- [ ] Check that all tests on the latest commit on `develop` or `breaking` branch (whichever was chosen for the prerelease) are green.
- [ ] Create a [release on GitHub](https://github.com/argotorg/solidity/releases/new).
    - Set the target to the `develop` or `breaking` branch and the tag to the new version with a prerelease suffix, e.g. `v0.8.5-pre.6`.
        Version matches the next release (`develop`) or the next breaking release (`breaking`).
        The prerelease number in the suffix is 1-based, sequential, resets after a full release and is counted separately for `develop` and `breaking`.
    - Include the following warning: `**The release is still in progress. You may see broken links and binaries may not yet be available from all sources.**`.
    - Include the current, incomplete changelog.
    - Check the `Set as a pre-release` box.
    - Click the `Publish release` button on the release page, creating the tag.
- [ ] Wait for the CI runs on the tag itself.
- [ ] Take the source tarball (`solidity_x.x.x-pre.N.tar.gz`) from `c_source_tarball` run of the tagged commit on Circle CI and upload it to the release page.
- [ ] Take the `github-binaries.tar` tarball from `c_release_binaries` run of the tagged commit on Circle CI and add all binaries from it to the release page.
      Make sure it contains five binaries: `solc-windows.exe`, `solc-macos`, `solc-static-linux`, `solc-static-linux-arm` and `soljson.js`.
- [ ] Take the `solc-bin-binaries.tar` tarball from `c_release_binaries` run of the tagged commit on Circle CI and add all binaries from it to solc-bin.
- [ ] Run `npm install` if you've got a clean checkout of the solc-bin repo.
- [ ] Run `npm run update -- --reuse-hashes` in `solc-bin` and verify that the script has updated `list.js`, `list.txt` and `list.json` files correctly and that symlinks to the new release have been added in `solc-bin/wasm/` and `solc-bin/emscripten-wasm32/`.
- [ ] Create a pull request in solc-bin and merge.
- [ ] Remove "still in progress" warning from the [release notes](https://github.com/argotorg/solidity/releases).
- [ ] Mention it on [Twitter](https://twitter.com/solidity_lang).
- [ ] Mention it on [Fosstodon](https://fosstodon.org/@solidity/).
- [ ] Mention it on [`#solidity` channel on Matrix](https://matrix.to/#/#ethereum_solidity:gitter.im).
- [ ] Mention it on [`#solc-tooling`](https://matrix.to/#/#solc-tooling:matrix.org).
- [ ] If anything went wrong this time, mention it in [Learning from Past Releases](https://notes.argot.org/@solidity-release-mistakes).
- [ ] Lean back, wait for bug reports and repeat from step 1 :).
