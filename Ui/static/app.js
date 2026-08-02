function showToast(message, kind = "info") {
    const toast = document.getElementById("toast");
    if (!toast) return;
    toast.textContent = message;
    toast.dataset.kind = kind;
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
            showToast("Please complete the required fields.", "error");
            return;
        }
        const result = document.getElementById(resultId);
        if (result) {
            result.textContent = description;
            result.classList.add("visible");
        }
        showToast("Interface action prepared.", "success");
    });
}

document.addEventListener("DOMContentLoaded", () => {
    const forms = [
        ["user-search-form", "user-search-result", "The user result card is ready for project data."],
        ["mutual-form", "mutual-result", "The mutual-friends result area is ready for project data."],
        ["path-form", "path-result", "The path visualization is ready for project data."],
        ["distance-form", "distance-result", "The distance table is ready for project data."],
        ["add-user-form", "add-user-result", "The add-user form passed interface validation."],
        ["edit-user-form", "edit-user-result", "The edit-user form passed interface validation."],
        ["remove-user-form", "remove-user-result", "The remove-user form passed interface validation."],
        ["add-friendship-form", "add-friendship-result", "The add-friendship form passed interface validation."],
        ["remove-friendship-form", "remove-friendship-result", "The remove-friendship form passed interface validation."]
    ];
    forms.forEach((item) => showPreparedResult(...item));

    const loadGroups = document.getElementById("load-groups");
    const groupsResult = document.getElementById("groups-result");
    if (loadGroups && groupsResult) {
        loadGroups.addEventListener("click", () => {
            groupsResult.innerHTML = '<div class="empty-state bordered">Connected-component cards will be inserted in this section.</div>';
            showToast("Group view refreshed.", "success");
        });
    }

    document.getElementById("refresh-user-table")?.addEventListener("click", () => {
        showToast("The user table layout is ready for data.", "success");
    });
});
