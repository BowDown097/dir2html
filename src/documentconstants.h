#pragma once
#include <string_view>

namespace DocumentConstants
{
constexpr std::string_view AudioIcon = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABAEAQAAABQ8GUWAAAAIGNIUk0AAHomAACAhAAA+gAAAIDoAAB1MAAA6mAAADqYAAAXcJy6UTwAAAACYktHRAAAqo0jMgAAAAlwSFlzAAAAYAAAAGAA8GtCzwAAAAd0SU1FB+kIEgMpMFcmJSEAAAM0SURBVHja7ZlNSNtgGMf/qRU11o8EREQsDsWPWjYPHupuA8euOwycUJm3jYljN8Wj7ORBBHfy4EVUxg7D82DeVNCDTIp204OWoTKp2NZUUPruEMpijM13Q5P3ByG8H0/6vP88z5O3CUChUCgUimdhlDoPDghx2jEraG9nGLU5fmUBnHa9ePicdsBpqABOO+A0ijXAHSVQG56PAM8LQFPAaQfsJP5jcNDTAvi+LSzsPQ+HdQtAiDsOfGBZ/6uVle3t8nJdArgFQgDyLByuZcfHPSlAHubjxMQvvrlZacyWp0B/vzn7jQ1rBSCfq6sxNDmJ5OiofMwTEQAAzNORkd9jDQ3yfr+Ri6lBCLC5acw2ErFHADLGsuR6eBhzMzPSfs9EAAAwf6JReZ9tO0Ezd9K2nWiut3evqqmpO3tyUlAAKzBayMwWUCn3hGxnGN/YwACmFxdtF6BY1NQA9fVARQXg05DQpDwSAVQEsCIEzVxDq21jo7h4PTBPOjqk7ZItgrW1+hcPAHjR1uYKAQwtHgB66uqkzZJNgcpKg9f+GQiAVxGgVLDiRjkqAMcBwSBQVQVks8DxsXbb62uDUeBPp6VNx2oAxwGhEBAIAGVl4jkU0m6fTBr7XWb38vKOHkqTilEDgkHjtgCQSomRw3E6/Xp5eKgqgB54HmhpAVgWEAQgkdBexJTm6RH/7ExMHZ4XN0Lqn0IBchCPQ/IcMCUAxwHd3f/bgcDddiEEQdzFyclk9PmQSomHEl1d9/vKvqyvS9um3gmqhXGhI5FQtstHkCXvBOXECbn5vramKoBWHqrCuZy6bTIJxGJAOg3c3ornWMx4cdNyE7C7tdXz9fRUOsdUCmSzymF8daXN/uJCPIrG36UleZepFFB6bhMCHB05/0r8Xgq8EwQmtLws99dUBOTDWL6ZKepd1QiZm5/vfHx+bqkAeRGszFtbFs9nMkzr9LTSmDc+jn6ampK+BpNSsn+HNfN6Z0d4Mzv70LC7BWjNZHzvh4b6+m5uPClA7jQa7Xy7v19ojqtrQM+j1VW1Oa6OAC14XgBXp4AWPB8BnheApoDTDjiNYgTEYk67RaFQKBQKpQj8A/PI+76fy059AAAAJXRFWHRkYXRlOmNyZWF0ZQAyMDI1LTA4LTEyVDEzOjQ2OjM1KzAwOjAwDwfErgAAACV0RVh0ZGF0ZTptb2RpZnkAMjAyNS0wOC0xMVQxNDozODowNyswMDowMAQD/UIAAAAodEVYdGRhdGU6dGltZXN0YW1wADIwMjUtMDgtMThUMDM6NDE6NDgrMDA6MDDJPXU3AAAAAElFTkSuQmCC";
constexpr std::string_view GenericIcon = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABAEAQAAABQ8GUWAAAAIGNIUk0AAHomAACAhAAA+gAAAIDoAAB1MAAA6mAAADqYAAAXcJy6UTwAAAACYktHRAAAqo0jMgAAAAlwSFlzAAAAYAAAAGAA8GtCzwAAAAd0SU1FB+kIEgQLCare/YwAAAI4SURBVHja7ZpNTsJAFIDfFMo/hMQYE+MB3HAcafEAegwXnsCliXvgGLo0caUu3CPBuCGEnwCldTGSDFJwpkz72ul8SVMglLz3zbw3AwVAo0k1hPeN30+rlXduGFEH6DS63dMv2w7r87PcAh4ByHHU6QMcvVpWvwEQlgShEfW86A8AAPPNsvonnQ66AEzCkpAYAWFJSEQJ/JXQu5MngVuAt5Lp/TDyl/IkJKIE/GZGrmVZn8N2W3kBfmWwxlzY9qESuAWQj32h4GEubLt3HVwC90YIYP9oYJK7te0eAJzdt1qi18a+BIQkBJgJyggIKoF/GXSw0wtHAve3wfcLx/FuMpmoExoO6UEIgGFsnvcdg0E222yu/t29CDVBDOp1mtBo5J/oml2vSxWAtQrUavQ8HtPRZ2FnBCtCugBviZM8K4EQgMlk92izpSFdQByoVmly0yl9zkpYzwz2sXICAAAqFZrkbLY9Cwxjsxx4SOQ+oFwGKJXoY3YGiCS+JhFN0I9ikSY8n2/3hFBKII4boUKBJrxcBlsChQTElXyejjgrQekm6Idp0sQdR7wXKCEAACD7m4nrCl6HHbhsCa4r1qwTuwrsQrQRKvd1WJREboS0AC1AHso1QVF0E8QOABstADsAbFIvQK8C3MnrVUBNtADsALAR+lE01U1QVbQA7ACw0QKwA8BGrwLYAWCjBWAHgE3qBegmiB0ANqkXwF0CLw+EwDN2uPz0r/juEQvcSA7yHyxMVOxYGvn8AASbEyrAjwtqAAAAJXRFWHRkYXRlOmNyZWF0ZQAyMDI1LTA4LTEyVDEzOjQ2OjM1KzAwOjAwDwfErgAAACV0RVh0ZGF0ZTptb2RpZnkAMjAyNS0wOC0xMVQxNDozODowNyswMDowMAQD/UIAAAAodEVYdGRhdGU6dGltZXN0YW1wADIwMjUtMDgtMThUMDQ6MTE6MDkrMDA6MDDnJNpjAAAAAElFTkSuQmCC";
constexpr std::string_view ImageIcon = "data:image/png;base64,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";
constexpr std::string_view VideoIcon = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAQAAAAAYLlVAAAAIGNIUk0AAHomAACAhAAA+gAAAIDoAAB1MAAA6mAAADqYAAAXcJy6UTwAAAACYktHRAAAqo0jMgAAAAlwSFlzAAAAYAAAAGAA8GtCzwAAAAd0SU1FB+kIEgQrH8uObH8AAALNSURBVGje7ZffSxRRFMc/q+KyGlrqQ+KoUSiuFRQYRFBIP/Slhx4jgij6M3rsrwgpjKhee+hB2B4iCqImszWzNHF1RTPJ0lxXcdweHNe7c2d25m44UzDffZi5d8+933O/5xzOHQgRIkSIECFChAgYka3HQM5/6t4IQMXWYCgwBSp2Xi+bz8e7SCdzmA6IEfAjGjscZT6wFYUQAn9PbnEgBzzywQGZ418JQQ64Yk493EU6mSOsghIViKKhUUM1BlmWSJNmrSQFhDJ84HFxOXHizDHJAqvkiFJLM0cZ5xOGy1qZowJFVNHNKgMs5WeyZPnGEEfo4TkZxf2EEFz1oEQVvUwwZBOmdd7RxFmeFXVB5lBKwnK6SfHeMUtm0DlDuZICZdsK7Gyac/zFMdDN9zoiNhYzzNJRZAeRQ1mBSjrR8wvbuUTb9n1GQJKDVCooIOTAfRfTZpaZF8bVnKQNnbkCqw1SaHx12EPmUFBAY0qaq6eHC+wrmJtCK02Ba+ZUv4PpXr4URHEbjVwkhc5vc/yDWsc0lTkU7gMxS4GJ9q1ojJBkA8jZZIYzh1IZRor8Z7DOpgc7KxR6QYaYw3+bjDNINn8mw1VN217Q77LoJw2kbeanecOyMK7nl+MeMoeNAk6YppNByxkWeGspQ2hhWuFOIThw3Zy652Ca4gRNggYr6ExIVFEO8MTRAZlDoRuukaSLWbPljvIqn3QijjPKuvdN1XrBMAanzPfvGDYWh2jgY6m9wGokwyBBI6cd+107x0gUvZTIHGbJ3vKcNdWcI8JrKfX20EUNCYULyW3x69g7VnjKYc6zyCTzrLBJjDpa2M8wL1yvZDIEB26Yz7suSww+8JlmWumgCsiwyBQvPaWezFHirTjLGGPKp7Xj+F+/C/4Wtr2gzwdimUOhF+wOBAVu+qCEzBF4DgReBYE7IITgjg90MofpwEjQQoQIDn8A1WojiBzO0VEAAAAldEVYdGRhdGU6Y3JlYXRlADIwMjUtMDgtMTJUMTM6NDY6MzUrMDA6MDAPB8SuAAAAJXRFWHRkYXRlOm1vZGlmeQAyMDI1LTA4LTExVDE0OjM4OjA3KzAwOjAwBAP9QgAAACh0RVh0ZGF0ZTp0aW1lc3RhbXAAMjAyNS0wOC0xOFQwNDo0MzozMSswMDowMC/m75gAAAAASUVORK5CYII=";

constexpr std::string_view ScriptBody = R"(
document.querySelectorAll(".metadata-toggle").forEach(toggle => {
    toggle.addEventListener("click", () => {
        const card = toggle.closest(".file-card");
        const panel = card.querySelector(".metadata-panel");
        const hidden = panel.style.height === "0px";
        panel.style.height = hidden ? `${panel.scrollHeight}px` : "0px";
        toggle.textContent = hidden ? '▲' : '▼';
    });
});)";
constexpr std::string_view StyleSheet = R"(
body {
    background-color: #222;
    color: white;
    font-family: "Open Sans", sans-serif;
    font-size: 14px;
    margin: auto;
}

.file-grid {
    display: flex;
    flex-wrap: wrap;
    gap: 2px;
}

.file-card {
    padding: 5px;
    box-sizing: border-box;
    width: 150px;
    display: flex;
    flex-direction: column;
}

.file-img {
    height: 64px;
    max-width: 100%;
    object-fit: contain;
    display: block;
    margin: 0 auto;
}

.file-info {
    margin-top: 5px;
    display: flex;
    position: relative;
}

.file-name {
    flex: 1 0;
    text-align: center;
    word-wrap: break-word;
    display: -webkit-box;
    -webkit-box-orient: vertical;
    -webkit-line-clamp: 3;
    overflow: hidden;
    text-overflow: ellipsis;
}

.metadata-toggle {
    cursor: pointer;
    font-size: 10px;
    user-select: none;
    align-self: end;
}

.metadata-panel {
    position: absolute;
    top: 100%;
    background-color: #111;
    z-index: 1;
    box-shadow: 0 4px 8px rgba(0,0,0,0.4);
    border-radius: 4px;
    padding: 0 4px;
    transition: height 0.3s ease;
    overflow: hidden;
    width: 100%;
})";
}
