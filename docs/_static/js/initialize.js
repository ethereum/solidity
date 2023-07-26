const getLogoSrc = (isDark) => (isDark ? DARK_LOGO_PATH : LIGHT_LOGO_PATH);

const getModeIconSrc = (isDark) => (isDark ? SUN_ICON_PATH : MOON_ICON_PATH);

const getMenuIconSrc = (isDark) =>
  isDark ? DARK_HAMBURGER_PATH : LIGHT_HAMBURGER_PATH;

function addFooterNote() {
  const contentInfo = document.querySelector("div[role=contentinfo]");
  const footerNote = document.createElement("p");
  footerNote.classList.add("footer-note");
  footerNote.innerHTML =
    'Customized with ❤️ by the <a href="https://ethereum.org/" target="_blank">ethereum.org</a> team.';
  contentInfo.parentNode.insertBefore(footerNote, contentInfo.nextSibling);
}

function rearrangeDom() {
  const bodyDivs = document.querySelectorAll("body>div");
  bodyDivs.forEach((div) => div.remove());
  const wrapperDiv = document.createElement("div");
  wrapperDiv.classList.add(WRAPPER_CLASS);
  bodyDivs.forEach((div) => wrapperDiv.appendChild(div));
  document.body.prepend(wrapperDiv);

  const rstVersions = document.querySelector(".rst-versions");
  rstVersions.remove();
  const wyNavSide = document.querySelector("nav.wy-nav-side");
  wyNavSide.appendChild(rstVersions);
  const backdrop = document.createElement("div");
  backdrop.classList.add("backdrop");
  wrapperDiv.appendChild(backdrop);

  const content = document.querySelector(".wy-nav-content");
  content.id = "content";
  const oldWrap = document.querySelector("section.wy-nav-content-wrap");
  oldWrap.remove();
  document.querySelector(".wy-grid-for-nav").appendChild(content);
}

function buildHeader() {
  const isDarkMode = localStorage.getItem(LS_COLOR_SCHEME) == DARK;

  const header = document.createElement("div");
  header.classList.add("unified-header");
  document.querySelector(`.${WRAPPER_CLASS}`).prepend(header);

  const innerHeader = document.createElement("div");
  innerHeader.classList.add("inner-header");
  header.appendChild(innerHeader);

  const homeLink = document.createElement("a");
  homeLink.classList.add("home-link");
  homeLink.href = SOLIDITY_HOME_URL;
  homeLink.ariaLabel = "Solidity home";
  innerHeader.appendChild(homeLink);

  const logo = document.createElement("img");
  logo.classList.add(SOLIDITY_LOGO_CLASS);
  logo.src = getLogoSrc(isDarkMode);
  logo.alt = "Solidity logo";
  homeLink.appendChild(logo);

  const skipToContent = document.createElement("a");
  skipToContent.classList.add("skip-to-content");
  skipToContent.href = "#content";
  skipToContent.innerText = "{skip to content}";
  innerHeader.appendChild(skipToContent);

  const navBar = document.createElement("nav");
  navBar.classList.add("nav-bar");
  innerHeader.appendChild(navBar);

  const linkElements = NAV_LINKS.map(({ name, href }) => {
    const link = document.createElement("a");
    link.classList.add("nav-link");
    link.setAttribute("key", name);
    link.setAttribute("href", href);
    link.setAttribute("aria-label", name);
    if (href === FORUM_URL) {
      link.classList.add("forum-link");
      link.setAttribute("target", "_blank");
      link.setAttribute("rel", "noopener noreferrer");
    }
    link.innerText = name;
    return link;
  });
  linkElements.forEach((link) => navBar.appendChild(link));

  // Flex wrapper for color mode and mobile menu buttons
  const navButtonContainer = document.createElement("div");
  navButtonContainer.classList.add("nav-button-container");
  navBar.appendChild(navButtonContainer);

  // Build color toggle
  const toggleIcon = document.createElement("img");
  toggleIcon.classList.add(COLOR_TOGGLE_ICON_CLASS);
  toggleIcon.src = getModeIconSrc(isDarkMode);
  toggleIcon.alt = "Color mode toggle icon";
  toggleIcon.setAttribute("aria-hidden", "true");
  toggleIcon.setAttribute("key", "toggle icon");
  const colorModeButton = document.createElement("button");
  colorModeButton.classList.add("color-toggle");
  colorModeButton.setAttribute("type", "button");
  colorModeButton.setAttribute("aria-label", "Toggle light dark mode");
  colorModeButton.setAttribute("key", "color mode button");
  colorModeButton.addEventListener("click", toggleColorMode);
  colorModeButton.appendChild(toggleIcon);
  navButtonContainer.appendChild(colorModeButton);

  // Build mobile hamburger menu
  const menuIcon = document.createElement("img");
  menuIcon.classList.add(COLOR_TOGGLE_ICON_CLASS);
  menuIcon.src = getMenuIconSrc(isDarkMode);
  menuIcon.alt = "Toggle menu";
  menuIcon.setAttribute("aria-hidden", "true");
  menuIcon.setAttribute("key", "menu icon");
  const menuButton = document.createElement("button");
  menuButton.classList.add("color-toggle");
  menuButton.classList.add("mobile-menu-button");
  menuButton.setAttribute("type", "button");
  menuButton.setAttribute("aria-label", "Toggle menu");
  menuButton.setAttribute("key", "menu button");
  menuButton.addEventListener("click", toggleMenu);
  menuButton.appendChild(menuIcon);
  navButtonContainer.appendChild(menuButton);
}

const updateActiveNavLink = () => {
  const navLinks = document.querySelectorAll(".unified-header .nav-link");
  navLinks.forEach((link) => {
    const href = link.getAttribute("href");
    if (document.documentURI.includes("contributing.html")) {
      link.classList[href.includes("contributing.html") ? "add" : "remove"](
        "active"
      );
    } else {
      link.classList[document.documentURI.includes(href) ? "add" : "remove"](
        "active"
      );
    }
  });
};

document.addEventListener("locationchange", updateActiveNavLink);

function updateGitHubEditPath() {
  // Replaces the version number in the GitHub edit path with "develop"
  const gitHubEditAnchor = document.querySelector(".wy-breadcrumbs-aside > a");
  const url = new URL(gitHubEditAnchor.href);
  const split = url.pathname.split("/");
  const versionIndex = split.indexOf("blob") + 1;
  split[versionIndex] = "develop";
  url.pathname = split.join("/");
  gitHubEditAnchor.setAttribute("href", url.toString());
  gitHubEditAnchor.setAttribute("target", "_blank");
  gitHubEditAnchor.setAttribute("rel", "noopener noreferrer");
}

function initialize() {
  // Rearrange DOM elements for styling
  rearrangeDom();

  // Check localStorage for existing color scheme preference
  var prefersDark = localStorage.getItem(LS_COLOR_SCHEME) == DARK;
  // Check link for search param "color"... it may be "light" or "dark"
  var urlParams = new URLSearchParams(window.location.search);
  if (urlParams.size > 0) {
    // This is used for color mode continuity between the main Solidity Lang site and the docs
    var colorSchemeParam = urlParams.get("color");
    // If present, overwrite prefersDark accordingly
    if (colorSchemeParam) {
      prefersDark = colorSchemeParam == DARK;
    }

    // Remove "color" search param from URL
    const { location, title } = document;
    const { pathname, origin, search, hash } = location;
    const newSearchParams = new URLSearchParams(search);
    newSearchParams.delete("color");
    const sanitizedSearch =
      newSearchParams.size < 1 ? "" : "?" + newSearchParams.toString();
    window.history.replaceState(
      origin,
      title,
      pathname + sanitizedSearch + hash
    );
  }

  // In case none existed, establish localStorage color scheme preference
  var mode = prefersDark ? DARK : LIGHT;
  localStorage.setItem(LS_COLOR_SCHEME, mode);

  // Select the root element and set the style attribute to denote color-scheme attribute
  document
    .querySelector(":root")
    .setAttribute("style", `--color-scheme: ${mode}`);

  // Remove old input and RTD logo anchor element
  document.querySelector("input[name=mode]").remove();
  document.querySelector("label[for=switch]").remove();
  document.querySelector(".wy-side-nav-search > a").remove();

  // Add footer note
  addFooterNote();

  // Build header
  buildHeader();

  // Close menu
  toggleMenu({ force: false });

  // Update active nav link
  updateActiveNavLink();

  // Update GitHub edit path to direct to `develop` branch
  updateGitHubEditPath();
}

document.addEventListener("DOMContentLoaded", initialize);

const handleClick = (e) => {
  if (e.target.closest(".backdrop")) {
    toggleMenu({ force: false });
  }

  if (e.target.closest("a")) {
    const target = e.target.closest("a");
    const href = target.getAttribute("href");
    if (href.includes(SOLIDITY_HOME_URL)) {
      const url = new URL(href);
      const params = new URLSearchParams(url.search);
      params.set("color", localStorage.getItem(LS_COLOR_SCHEME));
      url.search = params.toString();
      target.setAttribute("href", url.toString());
    }
  }
};
document.addEventListener("click", handleClick);

const handleKeyDown = (e) => {
  if (e.metaKey && e.key === "k") {
    document.querySelector("#rtd-search-form input").focus();
  } else if (e.key === "Escape") {
    toggleMenu({ force: false });
  }
  if (e.metaKey && e.code === "Backslash") {
    toggleColorMode();
  }
};
document.addEventListener("keydown", handleKeyDown);
