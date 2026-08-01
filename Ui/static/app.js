function showToast(message) {
    const toast = document.getElementById("toast");
    toast.textContent = message;
    toast.classList.add("visible");
    window.setTimeout(() => toast.classList.remove("visible"), 2600);
}

function showPreparedResult(formId, resultId, description) {
    const form = document.getElementById(formId);
    if (!form) return;

    form.addEventListener("submit", (event) => {
        event.preventDefault();
        const fields = [...form.querySelectorAll("input")];
        const missing = fields.some((field) => !field.value.trim());

        if (missing) {
            showToast("Please complete the required fields.");
            return;
        }

        const result = document.getElementById(resultId);
        result.textContent = description;
        result.classList.add("visible");
    });
}

document.addEventListener("DOMContentLoaded", () => {
    showPreparedResult(
        "user-search-form",
        "user-search-result",
        "The user result card is ready for real data."
    );
    showPreparedResult(
        "mutual-form",
        "mutual-result",
        "The mutual-friends result area is ready for real data."
    );
    showPreparedResult(
        "path-form",
        "path-result",
        "The path visualization is ready for the final command output."
    );
    showPreparedResult(
        "distance-form",
        "distance-result",
        "The distance table is ready for the final command output."
    );
});
