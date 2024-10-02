// This file was copied and adapted from: https://github.com/readthedocs/sphinx_rtd_theme/blob/master/sphinx_rtd_theme/static/js/versions.js_t
function renderLanguages(config) {
	if (!config.projects.translations.length) {
		return "";
	}

	const languagesHTML = `
		<dl>
			<dt> Languages </dt>
			${ config.projects.translations.map(
			(translation) => `
			<dd ${ translation.slug == config.projects.current.slug ? 'class="rtd-current-item"' : '' }>
				<a href="${ translation.urls.documentation }">${ translation.language.code }</a>
			</dd>
			`).join("\n")}
		</dl>
	`;
	return languagesHTML;
 }

function renderVersions(config) {
	if (!config.versions.active.length) {
		return "";
	}
	const versionsHTML = `
		<dl>
			<dt> Versions </dt>
			${ config.versions.active.map(
			(version) => `
			<dd ${ version.slug === config.versions.current.slug ? 'class="rtd-current-item"' : '' }>
				<a href="${ version.urls.documentation }">${ version.slug }</a>
			</dd>
			`).join("\n")}
		</dl>
	`;
	return versionsHTML;
}

function renderDownloads(config) {
	if (!Object.keys(config.versions.current.downloads).length) {
		return "";
	}
	const downloadsNameDisplay = {
		pdf: "PDF",
		epub: "Epub",
		htmlzip: "HTML",
	};

	const downloadsHTML = `
		<dl>
			<dt> Downloads </dt>
			${ Object.entries(config.versions.current.downloads).map(
			([name, url]) => `
				<dd>
				<a href="${ url }">${ downloadsNameDisplay[name] }</a>
				</dd>
			`).join("\n")}
		</dl>
	`;
	return downloadsHTML;
}

// NOTE: The Flyout and Search addons should be disabled in RTD dashboard to avoid conflicts with the custom flyout.
document.addEventListener("readthedocs-addons-data-ready", function(event) {
	const config = event.detail.data();
	console.log(config)

	const rstVersionsTemplate = document.createElement("template");
	rstVersionsTemplate.innerHTML = `
	<div class="rst-versions" data-toggle="rst-versions" role="note" arial-label="versions">
		<span class="rst-current-version" data-toggle="rst-current-version">
			<span class="fa fa-book">RTD</span>
			<span class="fa fa-element"></span>
			<span class="fa fa-v fa-element"> v: ${ config.versions.current.slug } <span class="fa fa-caret-down"></span></span>
		</span>

		<div class="rst-other-versions">
		<div>
			${ renderLanguages(config) }
			${ renderVersions(config) }
			${ renderDownloads(config) }
			<dl>
				<dt> On Read the Docs </dt>
				<dd>
					<a href="${ config.projects.current.urls.home }"> Project Home </a>
				</dd>
				<dd>
					<a href="${ config.projects.current.urls.builds }"> Builds </a>
				</dd>
				<dd>
					<a href="${ config.projects.current.urls.downloads }"> Downloads </a>
				</dd>
			</dl>
			<dl>
				<dt> Search </dt>
				<dd>
					<form id="flyout-search-form" class="wy-form" action="search.html" method="get">
					<input
						type="text"
						name="q"
						aria-label="Search docs"
						placeholder="Search docs"
						/>
					</form>
				</dd>
			</dl>
			<hr />
			<small>
			<span>Hosted by <a href="https://readthedocs.org">Read the Docs</a></span>
			<span> · </span>
			<a href="https://docs.readthedocs.io/page/privacy-policy.html">Privacy Policy</a>
			</small>
		</div>
		</div>
	`;
	const rstVersions = rstVersionsTemplate.content.firstElementChild;
	const wyNavSide = document.querySelector("nav.wy-nav-side");
	wyNavSide.appendChild(rstVersions);
});
