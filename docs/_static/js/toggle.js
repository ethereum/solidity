document.addEventListener('DOMContentLoaded', function() {

    function toggleCssMode(isDay) {
        var mode = (isDay ? "Day" : "Night");
        localStorage.setItem("css-mode", mode);

        var url_root = DOCUMENTATION_OPTIONS.URL_ROOT == "./" ? "" : DOCUMENTATION_OPTIONS.URL_ROOT;
        var daysheet = $(`link[href="${url_root}_static/pygments.css"]`)[0].sheet;
        daysheet.disabled = !isDay;

        var nightsheet = $(`link[href="${url_root}_static/css/dark.css"]`)[0];
        if (!isDay && nightsheet === undefined) {
            var element = document.createElement("link");
            element.setAttribute("rel", "stylesheet");
            element.setAttribute("type", "text/css");
            element.setAttribute("href", `${url_root}_static/css/dark.css`);
            document.getElementsByTagName("head")[0].appendChild(element);
            return;
        }
        if (nightsheet !== undefined) {
            nightsheet.sheet.disabled = isDay;
        }
    }

    var initial = localStorage.getItem("css-mode") != "Night";
    var checkbox = document.querySelector('input[name=mode]');

    toggleCssMode(initial);
    checkbox.checked = initial;

    checkbox.addEventListener('change', function() {
        document.documentElement.classList.add('transition');
        window.setTimeout(() => {
            document.documentElement.classList.remove('transition');
        }, 1000)
        toggleCssMode(this.checked);
    })

});